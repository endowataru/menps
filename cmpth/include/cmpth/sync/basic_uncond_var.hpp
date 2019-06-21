
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_uncond_var
{
    CMPTH_DEFINE_DERIVED(P)
    
    using worker_type = typename P::worker_type;
    using continuation_type = typename P::continuation_type;
    using task_desc_type = typename P::task_desc_type;
    
public:
    void notify(worker_type& wk)
    {
        CMPTH_P_ASSERT(P, this->cont_);
        
        wk.local_push_top(fdn::move(this->cont_));
    }
    
    worker_type& notify_enter(worker_type& wk)
    {
        CMPTH_P_ASSERT(P, this->cont_);
        
        return wk.template suspend_to_cont<
            basic_uncond_var::on_enter
        >
        (fdn::move(this->cont_));
    }
    
private:
    struct on_enter {
        worker_type& operator() (
            worker_type&        wk
        ,   continuation_type   cont
        ) {
            wk.local_push_top(fdn::move(cont));
            
            return wk;
        }
    };
    
public:
    worker_type& swap(worker_type& wk, derived_type& next_uv)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        CMPTH_P_ASSERT(P, next_uv.cont_);
        
        return wk.template suspend_to_cont<
            basic_uncond_var::on_swap
        >
        (fdn::move(next_uv.cont_), this);
    }
    
private:
    struct on_swap {
        worker_type& operator() (
            worker_type&            wk
        ,   continuation_type       cont
        ,   basic_uncond_var* const self
        ) {
            self->cont_ = fdn::move(cont);
            
            return wk;
        }
    };
    
public:
    template <typename Func, typename... Args>
    worker_type& swap_with(worker_type& wk, derived_type& next_uv,
        Args* const ... args)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        CMPTH_P_ASSERT(P, next_uv.cont_);
        
        return wk.template suspend_to_cont<
            basic_uncond_var::on_swap_with<Func, Args...>
        >
        (fdn::move(next_uv.cont_), this, args...);
    }
    
private:
    template <typename Func, typename... Args>
    struct on_swap_with {
        worker_type& operator() (
            worker_type&            wk
        ,   continuation_type       cont
        ,   basic_uncond_var* const self
        ,   Args* const ...         args
        ) {
            // Put the continuation to the uncond variable.
            // It may be used within Func.
            self->cont_ = fdn::move(cont);
            
            // Execute the user-defined function.
            if (Func{}(wk, args...)) {
                // Switch to "next_uv".
                return wk;
            }
            else {
                // The continuation must not be consumed.
                CMPTH_P_ASSERT(P, self->cont_);
                
                // Take the continuation to execute.
                cont = fdn::move(self->cont_);
                
                // Switch back to the original thread.
                wk.exit_to_cont_imm(fdn::move(cont));
            }
        }
    };
    
public:
    worker_type& wait(worker_type& wk)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        
        return wk.template suspend_to_sched<
            basic_uncond_var::on_swap // the same function
        >(this);
    }
    
public:
    template <typename Func, typename... Args>
    worker_type& wait_with(worker_type& wk, Args* const ... args)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        
        return wk.template suspend_to_sched<
            basic_uncond_var::on_wait_with<Func, Args...>
        >(this, args...);
    }
    
private:
    template <typename Func, typename... Args>
    struct on_wait_with {
        worker_type& operator() (
            worker_type&            wk
        ,   continuation_type       cont
        ,   basic_uncond_var* const self
        ,   Args* const ...         args
        ) {
            // Put the continuation to the uncond variable.
            // It may be used within Func.
            self->cont_ = fdn::move(cont);
            
            // Execute the user-defined function.
            if (Func{}(wk, args...)) {
                // Switch to "next_uv".
                return wk;
            }
            else {
                // The continuation must not be consumed.
                CMPTH_P_ASSERT(P, self->cont_);
                
                // Take the continuation to execute.
                cont = fdn::move(self->cont_);
                
                // Return to the original thread.
                wk.template cancel_suspend<
                    basic_uncond_var::on_wait_with_ret
                >
                (fdn::move(cont));
            }
        }
    };
    
    struct on_wait_with_ret {
        worker_type& operator() (
            worker_type&        wk
        ,   continuation_type   cont
        ) {
            if (cont) {
                // Return the popped task for the wait.
                wk.local_push_top(fdn::move(cont));
            }
            
            return wk;
        }
    };
    
public:
    // for end-users
    
    void notify() {
        auto& wk = worker_type::get_cur_worker();
        this->notify(wk);
    }
    void notify_signal() {
        this->notify();
    }
    void notify_enter() {
        auto& wk = worker_type::get_cur_worker();
        this->notify_enter(wk);
    }
    
    void swap(derived_type& next_uv) {
        auto& wk = worker_type::get_cur_worker();
        this->swap(wk, next_uv);
    }
    
    template <typename Func>
    void swap_with(derived_type& next_uv, Func func) {
        auto& wk = worker_type::get_cur_worker();
        this->template swap_with<on_call_instance<Func>>(wk, next_uv, &func);
    }
    
    void wait() {
        auto& wk = worker_type::get_cur_worker();
        this->wait(wk);
    }
    
    template <typename Func>
    void wait_with(Func func) {
        auto& wk = worker_type::get_cur_worker();
        this->template wait_with<on_call_instance<Func>>(wk, &func);
    }
    
private:
    template <typename Func>
    struct on_call_instance {
        CMPTH_NODISCARD
        bool operator() (
            worker_type&    /*wk*/
        ,   Func* const     func
        ) {
            // Copy the function here.
            auto f = *func;
            // Call the function on top of the next thread's stack.
            return f();
        }
    };
    
private:
    continuation_type   cont_;
};

} // namespace cmpth

