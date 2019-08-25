
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_suspended_thread
{
    CMPTH_DEFINE_DERIVED(P)
    
    using worker_type = typename P::worker_type;
    using continuation_type = typename P::continuation_type;
    using task_desc_type = typename P::task_desc_type;
    
public:
    basic_suspended_thread() noexcept = default;
    
    basic_suspended_thread(const basic_suspended_thread&) noexcept = delete;
    basic_suspended_thread& operator = (const basic_suspended_thread&) noexcept = delete;
    
    basic_suspended_thread(basic_suspended_thread&&) noexcept = default;
    basic_suspended_thread& operator = (basic_suspended_thread&&) noexcept = default;
    
    worker_type& wait() {
        auto& wk = worker_type::get_cur_worker();
        return this->wait(wk);
    }
    worker_type& wait(worker_type& wk)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        
        return wk.template suspend_to_sched<
            basic_suspended_thread::on_wait
        >(this);
    }
    
private:
    struct on_wait {
        worker_type& operator() (
            worker_type&                    wk
        ,   continuation_type               cont
        ,   basic_suspended_thread* const   self
        ) {
            self->cont_ = fdn::move(cont);
            return wk;
        }
    };
    
public:
    template <typename Func, typename... Args>
    worker_type& wait_with(Args* const ... args)
    {
        auto& wk = worker_type::get_cur_worker();
        return this->template wait_with<Func>(wk, args...);
    }
    template <typename Func, typename... Args>
    worker_type& wait_with(worker_type& wk, Args* const ... args)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        
        return wk.template suspend_to_sched<
            basic_suspended_thread::on_wait_with<Func, Args...>
        >(this, args...);
    }
    
private:
    template <typename Func, typename... Args>
    struct on_wait_with {
        worker_type& operator() (
            worker_type&        wk
        ,   continuation_type   prev_cont
        ,   derived_type* const self
        ,   Args* const ...     args
        ) {
            // Put the continuation to the previous suspended thread.
            // It may be used within Func.
            self->cont_ = fdn::move(prev_cont);
            
            // Execute the user-defined function.
            if (Func{}(wk, args...)) {
                // Switch to the popped thread.
                return wk;
            }
            else {
                // The continuation must not be consumed.
                CMPTH_P_ASSERT(P, self->cont_);
                
                // Take the continuation to execute.
                prev_cont = fdn::move(self->cont_);
                
                // Return to the original thread.
                wk.template cancel_suspend<
                    basic_suspended_thread::on_wait_with_fail
                >(fdn::move(prev_cont));
            }
        }
    };
    
    struct on_wait_with_fail {
        worker_type& operator() (
            worker_type&        wk
        ,   continuation_type   next_cont
        ) {
            if (next_cont) {
                // Return the popped task for the wait.
                wk.local_push_top(fdn::move(next_cont));
            }
            else {
                // The context was on top of the root task.
            }
            return wk;
        }
    };
    
public:
    void notify() {
        auto& wk = worker_type::get_cur_worker();
        this->notify(wk);
    }
    void notify(worker_type& wk)
    {
        CMPTH_P_ASSERT(P, this->cont_);
        
        wk.local_push_top(fdn::move(this->cont_));
    }
    
    worker_type& enter() {
        auto& wk = worker_type::get_cur_worker();
        return this->enter(wk);
    }
    worker_type& enter(worker_type& wk)
    {
        CMPTH_P_ASSERT(P, this->cont_);
        
        return wk.template suspend_to_cont<
            basic_suspended_thread::on_enter
        >(fdn::move(this->cont_));
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
    worker_type& swap(derived_type& next_sth) {
        auto& wk = worker_type::get_cur_worker();
        return this->swap(wk, next_sth);
    }
    worker_type& swap(worker_type& wk, derived_type& next_sth)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        CMPTH_P_ASSERT(P, next_sth.cont_);
        
        return wk.template suspend_to_cont<
            basic_suspended_thread::on_swap
        >(fdn::move(next_sth.cont_), this);
    }
    
private:
    struct on_swap {
        worker_type& operator() (
            worker_type&                    wk
        ,   continuation_type               cont
        ,   basic_suspended_thread* const   self
        ) {
            self->cont_ = fdn::move(cont);
            return wk;
        }
    };
    
public:
    template <typename Func, typename... Args>
    worker_type& swap_with(derived_type& next_sth, Args* const ... args)
    {
        auto& wk = worker_type::get_cur_worker();
        return this->template swap_with<Func>(wk, next_sth, args...);
    }
    
    template <typename Func, typename... Args>
    worker_type& swap_with(worker_type& wk, derived_type& next_sth,
        Args* const ... args)
    {
        CMPTH_P_ASSERT(P, !this->cont_);
        CMPTH_P_ASSERT(P, next_sth.cont_);
        
        return wk.template suspend_to_cont<
            basic_suspended_thread::on_swap_with<Func, Args...>
        >(fdn::move(next_sth.cont_), this, &next_sth, args...);
    }
    
private:
    template <typename Func, typename... Args>
    struct on_swap_with {
        worker_type& operator() (
            worker_type&                    wk
        ,   continuation_type               prev_cont
        ,   basic_suspended_thread* const   self
        ,   basic_suspended_thread* const   next_sth
        ,   Args* const ...                 args
        ) {
            // Put the continuation to the previous suspended thread.
            // It may be used within Func.
            self->cont_ = fdn::move(prev_cont);
            
            // Execute the user-defined function.
            if (Func{}(wk, args...)) {
                // Switch to "next_sth".
                return wk;
            }
            else {
                // The continuation must not be consumed.
                CMPTH_P_ASSERT(P, self->cont_);
                
                // Take the continuation to execute.
                prev_cont = fdn::move(self->cont_);
                
                // Return to the original thread.
                wk.template cancel_suspend<
                    basic_suspended_thread::on_swap_with_fail
                >
                (fdn::move(prev_cont), next_sth);
            }
        }
    };
    
    struct on_swap_with_fail {
        worker_type& operator() (
            worker_type&                    wk
        ,   continuation_type               next_cont
        ,   basic_suspended_thread* const   next_sth
        ) {
            // The continuation must not be the root context.
            CMPTH_P_ASSERT(P, next_cont);
            
            // Return the context to the original suspended thread.
            next_sth->cont_ = fdn::move(next_cont);
            
            return wk;
        }
    };
    
public:
    explicit operator bool() const noexcept {
        return !!this->cont_;
    }
    
private:
    continuation_type cont_;
};

} // namespace cmpth

