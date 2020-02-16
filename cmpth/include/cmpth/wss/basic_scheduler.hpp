
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_scheduler
{
    CMPTH_DEFINE_DERIVED(P)
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using base_thread_type = typename base_ult_itf_type::thread;
    
    using worker_type = typename P::worker_type;
    using worker_num_type = typename worker_type::worker_num_type;
    
    using task_pool_type = typename P::task_pool_type;
    
    using thread_funcs_type = typename P::thread_funcs_type;
    
    using tls_key_pool_type = typename P::tls_key_pool_type;
    
private:
    void loop(const worker_num_type wk_num)
    {
        auto& self = this->derived();
        auto& wk = self.wks_[wk_num];
        
        while (CMPTH_LIKELY(!self.is_finished()))
        {
            auto cont = wk.try_local_pop_top();
            
            if (cont) {
                // There is a local runnable task.
            }
            else {
                // Try to steal from other workers.
                cont = self.try_steal_from_others(wk);
                if (!cont) {
                    continue;
                }
            }
            
            CMPTH_P_LOG_DEBUG(P
            ,   "Execute task."
            ,   "wk_num", wk_num
            );
            
            wk.execute(fdn::move(cont));
        }
    }
    
    static void loop_child(derived_type* const self, const worker_num_type wk_num)
    {
        CMPTH_P_ASSERT(P, wk_num > 0);
        auto& wk = self->wks_[wk_num];
        
        wk.initialize_tls();
        
        self->loop(wk_num);
        
        wk.finalize_tls();
    }
    
    void start_workers()
    {
        auto& self = this->derived();
        const auto num_wks = this->num_wks_;
        
        for (worker_num_type wk_num = 1; wk_num < num_wks; ++wk_num) {
            this->ths_[wk_num] =
                base_thread_type{ &basic_scheduler::loop_child, &self, wk_num };
        }
    }
    
    void finish_workers()
    {
        const auto num_wks = this->num_wks_;
        for (worker_num_type wk_num = 1; wk_num < num_wks; ++wk_num) {
            this->ths_[wk_num].join();
        }
    }
    
public:
    void make_workers(const worker_num_type num_wks)
    {
        this->num_wks_ = num_wks;
        this->ths_ = fdn::make_unique<base_thread_type []>(num_wks);
        this->wks_ = fdn::make_unique<worker_type []>(num_wks);
        this->pool_ = fdn::make_unique<task_pool_type>(num_wks);
        
        for (worker_num_type wk_num = 0; wk_num < num_wks; ++wk_num) {
            this->wks_[wk_num].set_worker_num(wk_num);
        }
    }
    
    // child-first initialization
    class initializer;
    
private:
    struct fork_result
    {
        worker_type&    wk;
        task_pool_type& pool;
    };
    
    template <typename Func>
    struct on_execute_workers
    {
        derived_type& self;
        Func func;
        
        fork_result operator() () {
            func();
            
            this->self.set_finished();
            
            auto& wk = worker_type::get_cur_worker();
            auto& pool = this->self.get_task_pool();
            return { wk, pool };
        }
    };
    
public:
    // parent-first initialization
    template <typename Func>
    void execute_workers(Func&& func)
    {
        auto& self = this->derived();
        auto& root_wk = this->wks_[0];
        
        thread_funcs_type::fork_parent_first(
            root_wk
        ,   this->pool_->allocate(root_wk)
        ,   on_execute_workers<fdn::decay_t<Func>> {
                self, fdn::forward<Func>(func)
            }
        );
        
        this->start_workers();
        
        root_wk.initialize_tls();
        
        this->loop(0);
        
        root_wk.finalize_tls();
        
        this->finish_workers();
    }
    
    worker_type& get_worker_of_num(const worker_num_type wk_num) {
        //CMPTH_P_ASSERT(P, wk_num >= 0); // TODO
        CMPTH_P_ASSERT(P, wk_num < this->num_wks_);
        return this->wks_[wk_num];
    }
    
    worker_num_type get_num_workers() const noexcept {
        return this->num_wks_;
    }
    
    task_pool_type& get_task_pool() noexcept {
        return *this->pool_;
    }
    tls_key_pool_type& get_tls_key_pool() noexcept {
        return this->key_pool_;
    }
    
private:
    worker_num_type                         num_wks_ = 0;
    fdn::unique_ptr<base_thread_type []>    ths_;
    fdn::unique_ptr<worker_type []>         wks_;
    fdn::unique_ptr<task_pool_type>         pool_;
    tls_key_pool_type                       key_pool_;
};

template <typename P>
class basic_scheduler<P>::initializer
{
    using continuation_type = typename P::continuation_type;
    using task_ref_type = typename P::task_ref_type;
    
public:
    explicit initializer(derived_type& sched)
        : sched_(sched)
        // Note: GCC 4.8 cannot use {} for initializing a reference
    {
        this->sched_.start_workers();
        
        auto& root_wk = this->sched_.get_worker_of_num(0);
        
        root_wk.initialize_tls();
        
        // Extract the continuation here as a task.
        thread_funcs_type::fork_child_first(
            root_wk
        ,   sched.pool_->allocate(root_wk)
        ,   on_fork_root{ *this }
        );
        
        // The rest here is main function...
    }
    
private:
    struct on_fork_root
    {
        initializer& self;
        
        [[noreturn]]
        fork_result operator() () {
            // The forked ULT executes the scheduler loop here.
            self.sched_.loop(0);
            
            // Join the child workers.
            // This ULT is not executed on the children
            // because neither fork_child_first() nor loop() changes the worker.
            self.sched_.finish_workers();
            // self.dest_cont_ may be updated by one of the children.
            
            // The scheduler's context of the root is abandoned.
            auto& root_wk = self.sched_.get_worker_of_num(0);
            
            root_wk.template exit_to_cont<
                initializer::on_root_exit
            >
            (fdn::move(self.dest_cont_), &self);
            
            CMPTH_UNREACHABLE();
        }
    };
    
    struct on_root_exit {
        worker_type& operator() (
            worker_type&        wk
        ,   task_ref_type       tk
        ,   initializer* const  self
        ) const {
            // Deallocate the root task, which was running the scheduler loop.
            self->sched_.pool_->deallocate(wk, tk);
            
            return wk;
        }
    };
    
public:
    ~initializer()
    {
        // Notifies all of the workers.
        this->sched_.set_finished();
        
        // This worker may not be the root worker (wk_num = 0).
        auto& wk = worker_type::get_cur_worker();
        
        // We need to go back to the original (base ULT's) thread.
        auto& root_wk =
            wk.template suspend_to_sched<
                initializer::on_exit_root
            >(this);
        
        // The context here is resumed in the original thread.
        
        // Finalize the root worker.
        CMPTH_P_ASSERT(P, root_wk.get_worker_num() == 0);
        root_wk.finalize_tls();
    }
    
private:
    struct on_exit_root {
        worker_type& operator() (
            worker_type&        wk
        ,   continuation_type   cont
        ,   initializer* const  self
        ) const {
            // Save the context
            self->dest_cont_ = fdn::move(cont);
            
            // Return to the scheduler. It will immediately finish.
            return wk;
        }
    };
    
public:
    // uncopyable
    initializer(const initializer&) = delete;
    initializer& operator = (const initializer&) = delete;
    
private:
    derived_type&       sched_;
    continuation_type   dest_cont_;
};

} // namespace cmpth

