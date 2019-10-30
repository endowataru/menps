
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_worker_task
{
    CMPTH_DEFINE_DERIVED(P)
    
    using context_policy_type = typename P::context_policy_type;
    using context_type = typename P::context_type;
    using transfer_type = typename P::transfer_type;
    using cond_transfer_type = typename P::cond_transfer_type;
    
    using task_desc_type = typename P::task_desc_type;
    using call_stack_type = typename P::call_stack_type;
    using continuation_type = typename P::continuation_type;
    using running_task_type = typename P::running_task_type;
    using task_ref_type = typename P::task_ref_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    
    template <typename Func, typename... Args>
    struct on_suspend_to_new {
        transfer_type operator() (
            const context_type      ctx
        ,   derived_type* const     self_ptr
        ,   task_desc_type* const   next_desc
        ,   Args* const ...         args
        ) {
            auto& self = *self_ptr;
            self.check_cur_worker();
            
            // Set the context to the parent thread.
            auto prev_cont =
                self.cur_tk_.suspend_to(
                    ctx /*>---context---<*/, next_desc);
            
            auto& self_2 = Func{}(self, fdn::move(prev_cont), args...);
            
            self_2.check_cur_worker();
            return { &self_2 };
        }
    };
    
public:
    template <typename Func, typename... Args>
    derived_type& suspend_to_new(
        call_stack_type next_stk
    ,   Args* const ... args
    ) {
        auto& self = this->derived();
        self.check_cur_worker();
        
        const auto stack_ptr = next_stk.get_stack_ptr();
        const auto stack_size = next_stk.get_stack_size();
        
        // Release the ownership of the next thread.
        const auto next_desc = next_stk.release();
        
        const auto tr =
            context_policy_type::template save_context<
                basic_worker_task::on_suspend_to_new<Func, Args...>
            ,   derived_type
            >
            (stack_ptr, stack_size, &self, next_desc, args...);
        
        /*>---context---<*/
        
        auto& self_2 = *tr.p0;
        self_2.check_cur_worker();
        return self_2;
    }
    
private:
    template <typename Func, typename... Args>
    struct on_suspend_to_cont {
        transfer_type operator() (
            const context_type      ctx
        ,   derived_type* const     self_ptr
        ,   task_desc_type* const   next_desc
        ,   Args* const ...         args
        ) const {
            auto& self = *self_ptr;
            self.check_cur_worker();
            
            // Set the context to the parent thread.
            auto prev_cont =
                self.cur_tk_.suspend_to(
                    ctx /*>---context---<*/, next_desc);
            
            auto& self_2 = Func{}(self, fdn::move(prev_cont), args...);
            
            self_2.check_cur_worker();
            return { &self_2 };
        }
    };
    
public:
    template <typename Func, typename... Args>
    derived_type& suspend_to_cont(
        continuation_type   next_cont
    ,   Args* const ...     args
    ) {
        auto& self = this->derived();
        self.check_cur_worker();
        
        // Get the context of the following thread.
        const auto ctx = next_cont.get_context();
        
        // Release the ownership of the next thread.
        const auto next_desc = next_cont.release();
        
        const auto tr =
            context_policy_type::template swap_context<
                basic_worker_task::on_suspend_to_cont<Func, Args...>
            >
            (ctx, &self, next_desc, args...);
        
        /*>---context---<*/
        
        auto& self_2 = *tr.p0;
        self_2.check_cur_worker();
        return self_2;
    }
    
private:
    template <typename Func, typename... Args>
    struct on_cond_suspend_to_cont {
        cond_transfer_type operator() (
            const context_type          ctx
        ,   derived_type* const         self_ptr
        ,   continuation_type* const    next_cont
        ,   Args* const ...             args
        ) const {
            auto& self = *self_ptr;
            self.check_cur_worker();
            
            // Release the ownership of the next thread.
            const auto next_desc = next_cont->release();
            
            // Set the context to the parent thread.
            auto prev_cont =
                self.cur_tk_.suspend_to(
                    ctx /*>---context---<*/, next_desc);
            
            // Execute the user-defined function.
            auto& wk_2 = Func{}(self, prev_cont, args...);
            
            // When the continuation is consumed by Func,
            // Func requests this function to switch to next_cont.
            // Otherwise, it should return back to the original context.
            const bool flag = !prev_cont;
            CMPTH_P_LOG_VERBOSE(P, "Return from cond_suspend.", 1, "flag", flag);
            if (!flag) {
                // Revive the original continuation.
                *next_cont = wk_2.cur_tk_.revive_suspended(fdn::move(prev_cont));
            }
            
            wk_2.check_cur_worker();
            return { &wk_2, flag };
        }
    };
    
public:
    template <typename Func, typename... Args>
    derived_type& cond_suspend_to_cont(
        continuation_type&  next_cont
    ,   Args* const ...     args
    ) {
        auto& self = this->derived();
        self.check_cur_worker();
        
        // Get the context of the following thread.
        const auto ctx = next_cont.get_context();
        
        const auto tr =
            context_policy_type::template cond_swap_context<
                basic_worker_task::on_cond_suspend_to_cont<Func, Args...>
            >
            (ctx, &self, &next_cont, args...);
        
        /*>---context---<*/
        
        auto& self_2 = *tr.p0;
        self_2.check_cur_worker();
        return self_2;
    }
    
private:
    template <typename Func, typename... Args>
    struct on_exit {
        transfer_type operator() (
            derived_type* const     self_ptr
        ,   task_desc_type* const   next_desc
        ,   Args* const ...         args
        ) {
            auto& self = *self_ptr;
            self.check_cur_worker();
            
            auto prev_tk = self.cur_tk_.exit_to(next_desc);
            
            // Call the user-defined function.
            auto& self_2 = Func{}(self, prev_tk, args...);
            
            self_2.check_cur_worker();
            return { &self_2 };
        }
    };
    
public:
    template <typename Func, typename... Args>
    [[noreturn]]
    void exit_to_cont(
        continuation_type   next_cont
    ,   Args* const ...     args
    ) {
        auto& self = this->derived();
        self.check_cur_worker();
        
        // Get the context of the following thread.
        const auto ctx = next_cont.get_context();
        
        // Release the ownership of the next thread.
        const auto next_desc = next_cont.release();
        
        // Explicitly destroy the instance because it's not called automatically.
        next_cont.~continuation_type();
        
        // Switch to the context of the following thread.
        context_policy_type::template restore_context<
            basic_worker_task::on_exit<Func, Args...>
        >
        (ctx, &self, next_desc, args...);
        
        /*>--- this context is abandoned ---<*/
        
        // Be careful that the destructors are not called in this function.
        
        CMPTH_UNREACHABLE();
    }
    
    [[noreturn]]
    void exit_to_cont_imm(continuation_type next_cont) {
        this->exit_to_cont<on_exit_to_cont_imm>(fdn::move(next_cont));
    }
    
private:
    struct on_exit_to_cont_imm {
        derived_type& operator() (
            derived_type&   self
        ,   task_ref_type   /*prev_tk*/
            // Note: This task is discarded. The user is responsible to manage it.
        ) const {
            self.check_cur_worker();
            return self;
        }
    };
    
    // TODO: exit_to_new() ?
    
public:
    task_ref_type get_cur_task_ref() const noexcept {
        return this->cur_tk_.get_task_ref();
    }

protected:
    void mark_cur_task_as_root() {
        this->cur_tk_.mark_as_root();
    }
    void unmark_cur_task_as_root() {
        this->cur_tk_.unmark_as_root();
    }
    
private:
    running_task_type cur_tk_;
};

} // namespace cmpth

