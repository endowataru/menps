
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_worker
    : public P::worker_task_type
    , public P::worker_tls_type
{
    CMPTH_DEFINE_DERIVED(P)
    
    using worker_deque_type = typename P::worker_deque_type;
    
    using call_stack_type = typename P::call_stack_type;
    using continuation_type = typename P::continuation_type;
    
    using task_ref_type = typename P::task_ref_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    
public:
    using worker_num_type = typename P::worker_num_type;
    
    void local_push_top(continuation_type cont)
    {
        this->check_cur_worker();
        this->wd_.local_push_top( fdn::move(cont) );
    }
    void local_push_bottom(continuation_type cont)
    {
        this->check_cur_worker();
        this->wd_.local_push_bottom( fdn::move(cont) );
    }
    continuation_type try_local_pop_top()
    {
        this->check_cur_worker();
        return this->wd_.try_local_pop_top();
    }
    continuation_type try_remote_pop_bottom()
    {
        // This method is called from a thief worker.
        return this->wd_.try_remote_pop_bottom();
    }
    
    template <typename Func, typename... Args>
    derived_type& suspend_to_sched(Args* const ... args)
    {
        return this->template suspend_to_cont<Func>(
            this->local_pop_top_or_root(), args...);
    }
    template <typename Func, typename... Args>
    [[noreturn]]
    void exit_to_sched(Args* const ... args)
    {
        this->template exit_to_cont<Func>(
            this->local_pop_top_or_root(), args...);
    }
    
    template <typename Func, typename... Args>
    derived_type& cond_suspend_to_sched(Args* const ... args)
    {
        auto next_cont = this->local_pop_top_or_root();
        auto& wk_2 = this->template cond_suspend_to_cont<Func>(next_cont, args...);
        
        if (next_cont) {
            if (wk_2.root_cont_) {
                wk_2.local_push_top(fdn::move(next_cont));
            }
            else {
                wk_2.set_root_cont(fdn::move(next_cont));
            }
        }
        return wk_2;
    }
    
private:
    struct on_execute {
        derived_type& operator() (
            derived_type&       wk
        ,   continuation_type   cont
        ) {
            wk.check_cur_worker();
            
            // Set the root continuation.
            wk.set_root_cont(fdn::move(cont));
            
            return wk;
        }
    };
    
public:
    // Note: This function is used by the scheduler main loop.
    void execute(continuation_type cont)
    {
        CMPTH_P_ASSERT(P, !this->root_cont_);
        
        this->mark_cur_task_as_root();
    
        auto& wk CMPTH_MAYBE_UNUSED =
            this->template suspend_to_cont<
                basic_worker::on_execute
            >(fdn::move(cont));
        
        wk.check_cur_worker();
        CMPTH_P_ASSERT(P, &wk == this);

        this->unmark_cur_task_as_root();
        
        CMPTH_P_ASSERT(P, !this->root_cont_);
    }
    
private:
    struct on_yield {
        derived_type& operator() (
            derived_type&       wk
        ,   continuation_type   prev_cont
        ) {
            // Push the parent thread to the bottom.
            wk.local_push_bottom( fdn::move(prev_cont) );
            
            return wk;
        }
    };
    
public:
    derived_type& yield()
    {
        auto& wk_2 =
            this->template suspend_to_sched<
                basic_worker::on_yield
            >();
        
        wk_2.check_cur_worker();
        return wk_2;
    }
    
private:
    continuation_type local_pop_top_or_root()
    {
        if (auto cont = this->wd_.try_local_pop_top()) {
            return cont;
        }
        else {
            CMPTH_P_ASSERT(P, this->root_cont_);
            CMPTH_P_ASSERT(P, this->root_cont_.is_root());
            return fdn::move(this->root_cont_);
        }
    }
    
public:
    worker_num_type get_worker_num() const noexcept {
        CMPTH_P_ASSERT(P, this->wk_num_ != P::invalid_worker_num);
        return this->wk_num_;
    }
    void set_worker_num(const worker_num_type wk_num) {
        this->wk_num_ = wk_num;
    }
    
private:
    void set_root_cont(continuation_type root_cont)
    {
        CMPTH_P_ASSERT(P, root_cont);
        CMPTH_P_ASSERT(P, root_cont.is_root());
        CMPTH_P_ASSERT(P, !this->root_cont_);
        this->root_cont_ = fdn::move(root_cont);
    }
    
    worker_num_type     wk_num_ = P::invalid_worker_num;
    worker_deque_type   wd_;
    continuation_type   root_cont_;

    fdn::byte pad_[CMPTH_CACHE_LINE_SIZE * 2]; // TODO
};

} // namespace cmpth

