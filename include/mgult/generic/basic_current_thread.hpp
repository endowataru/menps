
#pragma once

#include <mgbase/crtp_base.hpp>

namespace mgult {

template <typename Policy>
class basic_current_thread
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::ult_ref_type   ult_ref_type;
    typedef typename Policy::ult_id_type    ult_id_type;
    
    typedef typename Policy::context_type   context_type;
    typedef typename Policy::transfer_type  transfer_type;
    
    typedef typename Policy::allocated_ult_type allocated_ult_type;
    
    typedef void (*fork_func_type)(void*);
    
private:
    struct suspend_data
    {
        derived_type&   self;
        ult_ref_type    parent_th;
        ult_ref_type    child_th;
        void*           ptr;
    };
    
    template <void (*Func)(derived_type&, ult_ref_type)>
    MGBASE_NORETURN MGCTX_SWITCH_FUNCTION
    static transfer_type suspend_to_new_handler(
        const context_type  ctx
    ,   suspend_data* const d
    ) {
        auto& self = d->self;
        self.check_current_worker();
        
        // Move the references to the child (current) thread.
        auto parent_th = mgbase::move(d->parent_th);
        auto child_th = mgbase::move(d->child_th);
        const auto ptr = d->ptr;
        
        // Call the hook.
        self.on_after_switch(parent_th, child_th);
        
        // Set the context to the parent thread.
        self.set_context(parent_th, ctx /*>---resuming context---<*/);
        
        // Change this thread to the child thread.
        self.set_current_ult(mgbase::move(child_th));
        
        Func(self, mgbase::move(parent_th), ptr);
        
        MGBASE_UNREACHABLE();
    }
    
public:
    template <void (*Func)(derived_type&, ult_ref_type)>
    derived_type& suspend_to_new(const allocated_ult_type& child, const fork_func_type func)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        typename derived_type::fork_stack_info info;
        
        self.setup_fork_info(child, func, &info);
        
        suspend_data d{
            self
        ,   self.remove_current_ult()
        ,   mgbase::move(info.child_th)
        ,   child.ptr
        };
        
        // Call the hook.
        self.on_before_switch(d.parent_th, d.child_th);
        
        const auto tr =
            self.save_context(
                info.stack_ptr
            ,   info.stack_size
            ,   MGBASE_NONTYPE_TEMPLATE(
                    &basic_current_thread::suspend_to_new_handler<Func>
                )
            ,   &d
            );
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        // Renew the worker.
        auto& self_2 = *tr.p0;
        self_2.check_current_worker();
        
        MGBASE_ASSERT(self_2.current_th_.is_valid());
        
        MGBASE_LOG_INFO(
            "msg:Suspended parent thread was resumed.\t"
            "{}"
        ,   self_2.show_ult_ref(self_2.current_th_)
        );
        
        return self_2;
    }
    
public:
    //derived_type& suspend_to_cont(ult_ref_type th);
    //derived_type& exit_to_cont(ult_ref_type);
    
    void check_current_ult_id(const ult_id_type& id)
    {
        Policy::check_ult_id(current_th_, id);
    }
    
public: // XXX
    void set_current_ult(ult_ref_type&& th)
    {
        MGBASE_ASSERT(th.is_valid());
        MGBASE_ASSERT(!current_th_.is_valid());
        
        MGBASE_LOG_INFO(
            "msg:Set the current thread.\t"
            "{}"
        ,   derived().show_ult_ref(th)
        );
        
        current_th_ = mgbase::move(th);
    }
    ult_ref_type remove_current_ult()
    {
        MGBASE_ASSERT(current_th_.is_valid());
        
        MGBASE_LOG_INFO(
            "msg:Remove the current thread.\t"
            "{}"
        ,   derived().show_ult_ref(current_th_)
        );
        
        return mgbase::move(current_th_);
    }
    
    // The current thread can be modified by the current worker
    // without acquiring a lock.
    ult_ref_type        current_th_;
};

} // namespace mgult

