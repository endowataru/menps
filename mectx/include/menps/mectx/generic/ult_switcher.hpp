
#pragma once

#include <menps/mectx/common.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/nontype.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mectx {

template <typename P>
class ult_switcher
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ult_ref_type = typename P::ult_ref_type;
    using context_type = typename P::context_type;
    using transfer_type = typename P::transfer_type;
    
private:
    template <typename Func>
    struct suspend_to_new_data
    {
        derived_type&   self;
        ult_ref_type&   prev_th;
        ult_ref_type&   next_th;
        Func            func;
    };
    
    template <typename Func>
    MEFDN_NORETURN MECTX_SWITCH_FUNCTION
    static transfer_type suspend_to_new_handler(
        const context_type                  ctx
    ,   suspend_to_new_data<Func>* const    d
    ) {
        auto& self = d->self;
        self.check_current_worker();
        
        // Set the context to the parent thread.
        self.set_context(d->prev_th, ctx /*>---resuming context---<*/);
        
        // Call the hook.
        self.on_after_switch(d->prev_th, d->next_th);
        
        // Move the user-defined function (in advance).
        auto func = mefdn::move(d->func);
        
        // Call the user-defined function.
        func(self, d->prev_th, d->next_th);
        
        MEFDN_UNREACHABLE();
    }
    
public:
    template <typename Func>
    derived_type& suspend_to_new(ult_ref_type& prev_th, ult_ref_type& next_th, Func func)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        suspend_to_new_data<Func> d{
            self
        ,   prev_th
        ,   next_th
        ,   func
        };
        
        // Call the hook.
        self.on_before_switch(d.prev_th, d.next_th);
        
        // Get the stack information.
        const auto stack_ptr = self.get_stack_ptr(d.next_th);
        const auto stack_size = self.get_stack_size(d.next_th);
        
        const auto tr =
            self.save_context(
                stack_ptr
            ,   stack_size
            ,   MEFDN_NONTYPE_TEMPLATE(
                    &ult_switcher::suspend_to_new_handler<Func>
                )
            ,   &d
            );
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        // Renew the worker.
        auto& self_2 = *tr.p0;
        self_2.check_current_worker();
        
        return self_2;
    }
    
private:
    template <typename Func>
    struct suspend_to_cont_data
    {
        derived_type&   self;
        ult_ref_type&   prev_th;
        ult_ref_type&   next_th;
        Func            func;
    };
    
    template <typename Func>
    MECTX_SWITCH_FUNCTION
    static transfer_type suspend_to_cont_handler(
        const context_type                  ctx
    ,   suspend_to_cont_data<Func>* const   d
    ) {
        auto& self = d->self;
        self.check_current_worker();
        
        // Set the context to the parent thread.
        self.set_context(d->prev_th, ctx /*>---resuming context---<*/);
        
        MEFDN_LOG_VERBOSE(
            "msg:Save context.\t"
            "ctx:0x{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(ctx.p)
        );
        
        // Call the hook.
        self.on_after_switch(d->prev_th, d->next_th);
        
        // Move the user-defined function (in advance).
        auto func = mefdn::move(d->func);
        
        // Call the user-defined function.
        return func(self, d->prev_th, d->next_th);
    }
    
public:
    template <typename Func>
    derived_type& suspend_to_cont(ult_ref_type& prev_th, ult_ref_type& next_th, Func func)
    {
        auto& self = this->derived();
        
        suspend_to_cont_data<Func> d{
            self
        ,   prev_th
        ,   next_th
        ,   func
        };
        
        // Call the hook.
        self.on_before_switch(d.prev_th, d.next_th);
        
        // Get the context of the thread executed next.
        const auto ctx = self.get_context(d.next_th);
        
        MEFDN_ASSERT(ctx.p != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Start to resume context.\t"
            "ctx:0x{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(ctx.p)
        );
        
        // Switch to the context of the next thread.
        const auto r =
            self.swap_context(
                ctx
            ,   MEFDN_NONTYPE_TEMPLATE(
                    &ult_switcher::suspend_to_cont_handler<Func>
                )
            ,   &d
            );
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        auto& self_2 = *r.p0;
        self_2.check_current_worker();
        
        return self_2;
    }
    
private:
    template <typename Func>
    struct exit_to_cont_data
    {
        derived_type&   self;
        ult_ref_type&   prev_th;
        ult_ref_type&   next_th;
        Func            func;
    };
    
    template <typename Func>
    MECTX_SWITCH_FUNCTION
    static transfer_type exit_to_cont_handler(exit_to_cont_data<Func>* const d)
    {
        auto& self = d->self;
        self.check_current_worker();
        
        // Call the hook.
        self.on_after_switch(d->prev_th, d->next_th);
        
        // Move the user-defined function (in advance).
        auto func = mefdn::move(d->func);
        
        // Call the user-defined function.
        return func(self, d->prev_th, d->next_th);
    }
    
public:
    template <typename Func>
    MEFDN_NORETURN
    void exit_to_cont(ult_ref_type& prev_th, ult_ref_type& next_th, Func func)
    {
        auto& self = this->derived();
        
        exit_to_cont_data<Func> d{
            self
        ,   prev_th
        ,   next_th
        ,   func
        };
        
        // Call the hook.
        self.on_before_switch(prev_th, d.next_th);
        
        // Get the context of the following thread.
        const auto ctx = self.get_context(d.next_th);
        
        MEFDN_ASSERT(ctx.p != nullptr);
        
        MEFDN_LOG_INFO(
            "msg:Exiting to continuation.\t"
            "{}"
        ,   self.show_ult_ref(d.next_th)
        );
        
        // Switch to the context of the following thread.
        self.restore_context(
            ctx
        ,   MEFDN_NONTYPE_TEMPLATE(
                &ult_switcher::exit_to_cont_handler<Func>
            )
        ,   &d
        );
        
        /*>--- this context is abandoned ---<*/
        
        // Be careful that the destructors are not called in this function.
        
        MEFDN_UNREACHABLE();
    }
};

} // namespace mectx
} // namespace menps

