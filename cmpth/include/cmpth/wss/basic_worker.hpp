
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
    
    using worker_num_type = typename P::worker_num_type;
    
public:
    void local_push_top(continuation_type cont)
    {
        this->wd_.local_push_top( fdn::move(cont) );
    }
    void local_push_bottom(continuation_type cont)
    {
        this->wd_.local_push_bottom( fdn::move(cont) );
    }
    continuation_type try_local_pop_top()
    {
        return this->wd_.try_local_pop_top();
    }
    continuation_type try_remote_pop_bottom()
    {
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
    
private:
    template <typename Func, typename... Args>
    struct on_cancel_suspend {
        derived_type& operator() (
            derived_type&       wk
        ,   const task_ref_type tk
        ,   Args* const ...     args
        ) {
            // Revive the exited thread.
            continuation_type cont{
                unique_task_ptr_type{ tk.get_task_desc() }
            };
            
            if (!wk.root_cont_) {
                // The revived task is the scheduler task.
                wk.root_cont_ = fdn::move(cont);
            }
            
            // Execute the user-defined function.
            // Note: cont may be null when it's the scheduler task.
            auto& self_2 = Func{}(wk, fdn::move(cont), args...);
            
            return self_2;
        }
    };
    
public:
    // Cancel the execution of an on-top function.
    template <typename Func, typename... Args>
    [[noreturn]]
    void cancel_suspend(
        continuation_type   cont
    ,   Args* const ...     args
    ) {
        this->template exit_to_cont<
            basic_worker::on_cancel_suspend<Func, Args...>
        >(fdn::move(cont));
    }
    
private:
    struct on_execute {
        derived_type& operator() (
            derived_type&       wk
        ,   continuation_type   cont
        ) {
            CMPTH_P_ASSERT(P, !wk.root_cont_);
            
            // Set the root context.
            wk.root_cont_ = fdn::move(cont);
            
            return wk;
        }
    };
    
public:
    // Note: This function is used by the scheduler main loop.
    void execute(continuation_type cont)
    {
        CMPTH_P_ASSERT(P, !this->root_cont_);
        
        this->template suspend_to_cont<
            basic_worker::on_execute
        >
        (fdn::move(cont));
        
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
    worker_num_type     wk_num_ = P::invalid_worker_num;
    worker_deque_type   wd_;
    continuation_type   root_cont_;
};

} // namespace cmpth

