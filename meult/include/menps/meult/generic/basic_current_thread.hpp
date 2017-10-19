
#pragma once

#include <menps/meult/common.hpp>

namespace menps {
namespace meult {

template <typename Policy>
class basic_current_thread
{
    MEFDN_DEFINE_DERIVED(Policy)
    
    typedef typename Policy::ult_ref_type   ult_ref_type;
    typedef typename Policy::ult_id_type    ult_id_type;
    
    typedef typename Policy::context_type   context_type;
    typedef typename Policy::transfer_type  transfer_type;
    
    typedef typename Policy::allocated_ult_type allocated_ult_type;
    
    typedef void (*fork_func_type)(void*);
    
private:
    struct suspend_to_new_data
    {
        derived_type&   self;
        ult_ref_type    parent_th;
        ult_ref_type    child_th;
    };
    
    template <void (*Func)(derived_type&, ult_ref_type)>
    MEFDN_NORETURN MECTX_SWITCH_FUNCTION
    static transfer_type suspend_to_new_handler(
        const context_type          ctx
    ,   suspend_to_new_data* const  d
    ) {
        auto& self = d->self;
        self.check_current_worker();
        
        // Move the references to the child (current) thread.
        auto parent_th = mefdn::move(d->parent_th);
        auto child_th = mefdn::move(d->child_th);
        
        // Set the context to the parent thread.
        self.set_context(parent_th, ctx /*>---resuming context---<*/);
        
        // Change this thread to the child thread.
        self.set_current_ult(mefdn::move(child_th));
        
        // Call the user-defined function.
        Func(self, mefdn::move(parent_th));
        
        MEFDN_UNREACHABLE();
    }
    
public:
    template <void (*Func)(derived_type&, ult_ref_type)>
    derived_type& suspend_to_new(const allocated_ult_type& child, const fork_func_type func)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        typename derived_type::fork_stack_info info;
        
        self.setup_fork_info(child, func, &info);
        
        suspend_to_new_data d{
            self
        ,   self.remove_current_ult()
        ,   mefdn::move(info.child_th)
        };
        
        // Call the hook.
        self.on_before_switch(d.parent_th, d.child_th);
        
        const auto tr =
            self.save_context(
                info.stack_ptr
            ,   info.stack_size
            ,   MEFDN_NONTYPE_TEMPLATE(
                    &basic_current_thread::suspend_to_new_handler<Func>
                )
            ,   &d
            );
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        MEFDN_ASSERT(!d.parent_th.is_valid());
        MEFDN_ASSERT(!d.child_th.is_valid());
        
        // Renew the worker.
        auto& self_2 = *tr.p0;
        self_2.check_current_worker();
        
        MEFDN_ASSERT(self_2.current_th_.is_valid());
        
        MEFDN_LOG_INFO(
            "msg:Suspended parent thread was resumed.\t"
            "{}"
        ,   self_2.show_ult_ref(self_2.current_th_)
        );
        
        return self_2;
    }
    
private:
    template <typename T>
    struct suspend_to_cont_data
    {
        derived_type&   self;
        ult_ref_type    prev_th;
        ult_ref_type    next_th;
        T*              ptr;
    };
    
    template <typename T, transfer_type (*Func)(derived_type&, ult_ref_type, T*), bool IsPrevLocked>
    MECTX_SWITCH_FUNCTION
    static transfer_type suspend_to_cont_handler(
        const context_type              ctx
    ,   suspend_to_cont_data<T>* const  d
    ) {
        auto& self = d->self;
        self.check_current_worker();
        
        // Move the references to the current thread.
        auto prev_th = mefdn::move(d->prev_th);
        auto next_th = mefdn::move(d->next_th);
        
        // Set the context to the parent thread.
        self.set_context(prev_th, ctx /*>---resuming context---<*/);
        
        // Change this thread to the child thread.
        self.set_current_ult(mefdn::move(next_th));
        
        // Call the user-defined function.
        return Func(self, mefdn::move(prev_th), d->ptr);
    }
    
public:
    template <typename T, transfer_type (*Func)(derived_type&, ult_ref_type, T*), bool IsPrevLocked>
    derived_type& suspend_to_cont(ult_ref_type next_th, T* const ptr)
    {
        auto& self = this->derived();
        
        suspend_to_cont_data<T> d{
            self
        ,   self.remove_current_ult()
        ,   mefdn::move(next_th)
        ,   ptr
        };
        
        // Call the hook.
        self.on_before_switch(d.prev_th, d.next_th);
        
        // Get the context of the thread executed next.
        const auto ctx = self.get_context(d.next_th);
        
        MEFDN_ASSERT(ctx.p != nullptr);
        
        // Switch to the context of the next thread.
        const auto r =
            self.swap_context(
                ctx
            ,   MEFDN_NONTYPE_TEMPLATE(
                    &basic_current_thread::suspend_to_cont_handler<T, Func, IsPrevLocked>
                )
            ,   &d
            );
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        MEFDN_ASSERT(!d.prev_th.is_valid());
        MEFDN_ASSERT(!d.next_th.is_valid());
        
        auto& self_2 = *r.p0;
        self_2.check_current_worker();
        
        MEFDN_ASSERT(self_2.current_th_.is_valid());
        
        MEFDN_LOG_INFO(
            "msg:Resumed the thread blocked to join a child thread that finished now.\t"
            "{}"
        ,   self_2.show_ult_ref(self_2.current_th_)
        );
        
        return self_2;
    }
    
private:
    struct exit_to_cont_data
    {
        derived_type&   self;
        ult_ref_type    next_th;
    };
    
    template <transfer_type (*Func)(derived_type&, ult_ref_type), bool IsPrevLocked>
    MECTX_SWITCH_FUNCTION
    static transfer_type exit_to_cont_handler(exit_to_cont_data* const d)
    {
        auto& self = d->self;
        self.check_current_worker();
        
        // Move the references to the current thread.
        auto prev_th = self.remove_current_ult();
        auto next_th = mefdn::move(d->next_th);
        
        // Change this thread to the child thread.
        self.set_current_ult(mefdn::move(next_th));
        
        // Call the user-defined function.
        return Func(self, mefdn::move(prev_th));
    }
    
public:
    template <transfer_type (*Func)(derived_type&, ult_ref_type), bool IsPrevLocked>
    MEFDN_NORETURN
    void exit_to_cont(ult_ref_type next_th)
    {
        auto& self = this->derived();
        
        exit_to_cont_data d{
            self
        ,   mefdn::move(next_th)
        };
        
        // Call the hook.
        self.on_before_switch(self.current_th_, d.next_th);
        
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
                &basic_current_thread::exit_to_cont_handler<Func, IsPrevLocked>
            )
        ,   &d
        );
        
        /*>--- this context is abandoned ---<*/
        
        // Be careful that the destructors are not called in this function.
        
        MEFDN_UNREACHABLE();
    }
    
    void check_current_ult_id(const ult_id_type& id)
    {
        Policy::check_ult_id(current_th_, id);
    }
    
    ult_id_type get_current_ult_id() const noexcept
    {
        return this->current_th_.get_id();
    }
    
    std::string show_current_ult()
    {
        auto& self = this->derived();
        
        return self.show_ult_ref(this->current_th_);
    }
    
public: // XXX
    void set_current_ult(ult_ref_type&& th)
    {
        MEFDN_ASSERT(th.is_valid());
        MEFDN_ASSERT(!current_th_.is_valid());
        
        MEFDN_LOG_INFO(
            "msg:Set the current thread.\t"
            "{}"
        ,   derived().show_ult_ref(th)
        );
        
        current_th_ = mefdn::move(th);
    }
    // TODO: privacy
    ult_ref_type& get_current_ult() noexcept
    {
        return current_th_;
    }
    ult_ref_type remove_current_ult()
    {
        MEFDN_ASSERT(current_th_.is_valid());
        
        MEFDN_LOG_INFO(
            "msg:Remove the current thread.\t"
            "{}"
        ,   derived().show_ult_ref(current_th_)
        );
        
        return mefdn::move(current_th_);
    }
    
private:
    // The current thread can be modified by the current worker
    // without acquiring a lock.
    ult_ref_type        current_th_;
};

} // namespace meult
} // namespace menps

