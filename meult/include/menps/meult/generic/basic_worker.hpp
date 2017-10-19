
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/memory/align_nocheck.hpp>
#include <menps/mefdn/memory/distance_in_bytes.hpp>
#include <menps/mefdn/nontype.hpp>
#include <menps/mefdn/mutex.hpp>
#include <menps/mectx/common.hpp>
#include <algorithm>

namespace menps {
namespace meult {

template <typename Policy>
class basic_worker
{
private:
    typedef typename Policy::derived_type           derived_type;
    typedef typename Policy::ult_ref_type           ult_ref_type;
    typedef typename Policy::worker_deque_type      worker_deque_type;
    typedef typename Policy::worker_deque_conf_type worker_deque_conf_type;
    typedef typename Policy::ult_id_type            ult_id_type;
    
    typedef void (*fork_func_type)(void*);
    
    typedef typename Policy::context_type   context_type;
    typedef typename Policy::transfer_type  transfer_type;
    
    typedef typename Policy::allocated_ult_type allocated_ult_type;
    
public:
    typedef allocated_ult_type  allocated_ult;
    
private:
    struct suspension_data
    {
        context_type    ctx;
        
        fork_func_type  func;   // used in fork
        void*           ptr;    // used in fork
        ult_id_type     id;     // used in fork (TODO: unnecessary in Release build?)
    };
    
    static void* align_suspension_data(void*& stack_ptr, mefdn::size_t& stack_size)
    {
        return mefdn::align_call_stack_nocheck(
            alignof(suspension_data)
        ,   sizeof(suspension_data)
        ,   stack_ptr
        ,   stack_size
        );
    }
    
    suspension_data& get_suspension_data(ult_ref_type& th)
    {
        auto stack_ptr  = th.get_stack_ptr();
        auto stack_size = th.get_stack_size();
        
        return *static_cast<suspension_data*>(
            align_suspension_data(stack_ptr, stack_size)
        );
    }
    
public: // XXX
    context_type get_context(ult_ref_type& th)
    {
        auto& d = this->get_suspension_data(th);
        return d.ctx;
    }
    void set_context(ult_ref_type& th, const context_type ctx)
    {
        auto& d = this->get_suspension_data(th);
        d.ctx = ctx;
    }
    
protected:
    explicit basic_worker(const worker_deque_conf_type& conf)
        : wd_(conf)
    { }
    
private:
    static transfer_type loop_root_handler(
        derived_type&   self
    ,   ult_ref_type    prev_th
    ,   void* const     /*unused*/
    ) {
        // Set the root thread.
        self.root_th_ = mefdn::move(prev_th);
        
        // Switch to the resumed context of the following thread.
        return { &self };
    }
    
public:
    void loop()
    {
        auto& self = this->derived();
        
        self.check_current_worker();
        
        // Allocate a thread descriptor for the root thread.
        self.set_current_ult(self.allocate_ult());
        
        while (MEFDN_LIKELY(!self.finished()))
        {
            auto th = this->try_pop_top();
            
            if (MEFDN_LIKELY(th.is_valid()))
            {
                MEFDN_LOG_INFO(
                    "msg:Popped a thread.\t"
                    "{}"
                ,   self.show_ult_ref(th)
                );
            }
            else
            {
                th = self.try_steal_from_another();
                
                if (MEFDN_LIKELY(th.is_valid()))
                {
                    MEFDN_LOG_INFO(
                        "msg:Stole thread in worker loop.\t"
                        "{}"
                    ,   self.show_ult_ref(th)
                    );
                }
                else
                {
                    MEFDN_LOG_DEBUG("msg:Failed to steal in worker loop.");
                    continue;
                }
            }
            
            MEFDN_ASSERT(th.is_valid());
            
            const auto root_id = self.get_current_ult_id();
            
            MEFDN_LOG_INFO(
                "msg:Set up a root thread. Resume the thread.\t"
                "{}"
            ,   self.show_current_ult()
            );
            
            // Suspend the root thread.
            auto& self_2 =
                self.template suspend_to_cont<void, &basic_worker::loop_root_handler, false>(
                    mefdn::move(th)
                ,   nullptr
                );
            
            MEFDN_ASSERT(&self == &self_2);
            self.check_current_worker();
            self.check_current_ult_id(root_id);
            MEFDN_ASSERT(!this->root_th_.is_valid());
            
            MEFDN_LOG_INFO(
                "msg:The thread finished. Came back to worker loop."
            );
            
        }
        
        // Deallocate the root thread.
        self.deallocate_ult(self.remove_current_ult());
        #if 0
        // TODO: necessary for MGTH_ENABLE_ASYNC_WRITE_BACK
        
        auto current_th = self.remove_current_ult();
        
        // Detach the thread to remove it.
        // If the write back is still ongoing, detach doesn't destroy it immediately.
        self.detach(current_th.get_id());
        #endif
    }
    
public:
    MEFDN_NODISCARD
    allocated_ult allocate(
        const mefdn::size_t    alignment
    ,   const mefdn::size_t    size
    )
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        // Allocate a new thread descriptor.
        auto th = self.allocate_ult();
        
        const auto id = th.get_id();
        
        // Although these two are modified by align_call_stack,
        // those modifications are discarded.
        void* stack_ptr = th.get_stack_ptr();
        mefdn::size_t stack_size = th.get_stack_size();
        
        // Allocate a space for suspension data.
        auto p = this->align_suspension_data(stack_ptr, stack_size);
        
        new (p) suspension_data();
        
        if (MEFDN_LIKELY(size > 0))
        {
            // Allocate a space for user-defined data.
            MEFDN_MAYBE_UNUSED
            const auto ptr =
                mefdn::align_call_stack_nocheck(
                    alignment
                ,   size
                ,   stack_ptr
                ,   stack_size
                );
            
            #ifndef MEFDN_DISABLE_ALIGN_CHECK_SIZE
            if (MEFDN_UNLIKELY(ptr == nullptr))
            {
                // The required size by the user is too big
                // to place on the call stack.
                throw std::bad_alloc{};
            }
            #endif
        }
        
        return { id, stack_ptr };
    }
    
public: // XXX
    struct fork_stack_info
    {
        ult_ref_type    child_th;
        void*           stack_ptr;
        mefdn::size_t  stack_size;
    };
    
    void setup_fork_info(
        const allocated_ult&    child
    ,   const fork_func_type    func
    ,   fork_stack_info* const  out
    ) {
        auto& self = this->derived();
        self.check_current_worker();
        
        // Get the reference to the child thread again.
        // (allocate() had it once.)
        // TODO: Reduce two look-ups for the child thread.
        auto child_th = self.get_ult_ref_from_id(child.id);
        
        auto orig_stack_ptr   = child_th.get_stack_ptr();
        auto orig_stack_size  = child_th.get_stack_size();
        
        const auto stack_ptr = child.ptr;
        
        const auto used_size =
            static_cast<mefdn::size_t>(
                mefdn::distance_in_bytes(stack_ptr, orig_stack_ptr)
                * MEFDN_CALL_STACK_GROW_DIR
            );
        
        out->child_th   = mefdn::move(child_th);
        out->stack_ptr  = stack_ptr;
        out->stack_size = orig_stack_size - used_size;
        
        auto& sus_data =
            * static_cast<suspension_data*>(
                align_suspension_data(orig_stack_ptr, orig_stack_size)
            );
        
        sus_data.func = func;
        sus_data.ptr  = child.ptr;
        sus_data.id   = child.id;
    }
    
private:
    MEFDN_NORETURN MECTX_SWITCH_FUNCTION
    static void fork_parent_first_handler(const transfer_type tr) noexcept
    {
        // Get the worker reference passed by the previous thread.
        auto& self = *tr.p0;
        
        // Get the reference to the fork data.
        auto& d = self.get_suspension_data(self.get_current_ult());
        
        const auto child_id = d.id;
        
        self.check_current_worker();
        self.check_current_ult_id(child_id);
        
        MEFDN_LOG_INFO(
            "msg:Child thread forked in a parent-first manner was resumed.\t"
            "{}"
        ,   self.show_current_ult()
        );
        
        // Execute the user-specified function on this stack.
        d.func(d.ptr);
        
        // Renew the worker again.
        auto& self_2 = derived_type::renew_worker(child_id);
        self_2.check_current_ult_id(child_id);
        
        // Destruct the data.
        //d.~fork_parent_first_data();
        
        // Exit this thread.
        self_2.exit();
        
        // Be careful that the destructors are not called in this function.
    }
    
public:
    void fork_parent_first(const allocated_ult& child, const fork_func_type func)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        fork_stack_info info;
        
        self.setup_fork_info(child, func, &info);
        
        // Prepare a context.
        const auto ctx =
            self.make_context(
                info.stack_ptr // The call stack starts from here.
            ,   info.stack_size
            ,   MEFDN_NONTYPE_TEMPLATE(&basic_worker::fork_parent_first_handler)
            );
        
        // Set the context.
        self.set_context(info.child_th, ctx);
        
        MEFDN_LOG_INFO(
            "msg:Parent thread forked in a parent-first manner is pushed.\t"
            "{}"
        ,   self.show_ult_ref(info.child_th)
        );
        
        // Push the child thread on the top.
        self.push_top( mefdn::move(info.child_th) );
    }
    
private:
    struct join_data
    {
        ult_ref_type    child_th;
    };
    
    static transfer_type join_handler(
        derived_type&       self
    ,   ult_ref_type        prev_th
    ,   join_data* const    d
    ) {
        {
            // Set this thread as blocked.
            // This is disabled because it seems unnecessary.
            //prev_th.set_blocked();
            
            // Move the reference to this call stack.
            auto child_th = mefdn::move(d->child_th);
            
            // Call the hook.
            // This hook cannot precede moving the reference of the child thread.
            self.template on_after_switch<false>(prev_th, self.get_current_ult());
            
            // This worker already has a lock of the child thread.
            auto lc = child_th.get_lock(mefdn::adopt_lock);
            
            MEFDN_LOG_INFO(
                "msg:Set the current thread as the joiner thread.\t"
                //"{}"
            //,   self.show_ult_ref(d->child_th)
            );
            
            // Set the blocking thread continued by the child thread.
            child_th.set_joiner(lc, mefdn::move(prev_th));
            
            // The child thread is automatically unlocked here.
        }
        
        // Switch to the resumed context of the following thread.
        return { &self };
    }
    
public:
    void join(const ult_id_type& id)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        // Now this thread is the parent thread.
        
        auto child_th = self.get_ult_ref_from_id(id);
        
        {
            // Lock the child thread to examine the state.
            auto lc = child_th.get_lock();
            
            // The child thread must not be detached
            // because the current thread is joining it.
            MEFDN_ASSERT(!child_th.is_detached(lc));
            
            if (MEFDN_LIKELY(child_th.is_finished(lc))) {
                // The child thread has already finished
                // (and has written back the cache on distributed-memory implementation).
                
                MEFDN_LOG_INFO(
                    "msg:Join a thread that already finished.\t"
                    "{}"
                ,   self.show_ult_ref(child_th)
                );
                
                // Call the hook to issue write/read barriers.
                // The joined thread may have modified data.
                self.on_join_already(self.get_current_ult(), child_th, lc);
                
                if (child_th.is_latest_stamp(lc)) {
                    // Unlock the child thread to destroy the thread descriptor.
                    lc.unlock();
                    
                    // Destroy the thread descriptor of the child thread.
                    // The thread is not detached because it is being joined,
                    //
                    // Both shared-memory and distributed-memory versions destroy this thread
                    // because the write back is already completed.
                    self.deallocate_ult( mefdn::move(child_th) );
                }
                else {
                    // Detach the thread.
                    // This execution path is disabled in shared-memory implementation.
                    child_th.set_detached(lc);
                }
                
                return;
            }
            else {
                MEFDN_LOG_INFO(
                    "msg:Joining a thread that is still running.\t"
                    "{}"
                ,   self.show_ult_ref(child_th)
                );
            }
            
            // Release the lock; this lock is unlocked by the handler.
            lc.release();
        }
        
        join_data d{
            mefdn::move(child_th)
        };
        
        // Get the next thread. It might be a root thread.
        auto next_th = self.pop_top();
        
        // Suspend the current thread.
        auto& self_2 =
            self.template suspend_to_cont<join_data, &basic_worker::join_handler, false>(
                mefdn::move(next_th)
            ,   &d
            );
        
        MEFDN_LOG_INFO(
            "msg:Resumed the thread blocked to join a child thread that finished now.\t"
            "{}"
        ,   self_2.show_current_ult()
        );
        
        // Get the reference to the child thread again
        // because this context is resumed on the worker of the child thread.
        
        // The reference on the stack must be invalid.
        MEFDN_ASSERT(!d.child_th.is_valid());
        
        auto child_th_2 = self_2.get_ult_ref_from_id(id);
        
        // In shared-memory implementation, no need to lock here
        // because the child thread has already finished
        // and no other threads are joining.
        
        #ifdef MEULT_CHECK_FINISHED_ULT
            // disabled by default
        {
            // Currently, we need to lock the descriptor
            // to check whether the thread has finished.
            auto lc = child_th_2.get_lock();
            
            // The child thread already finished
            // because this parent thread is joining.
            MEFDN_ASSERT(child_th_2.is_finished(lc));
            
            // If the other threads are joining this child thread,
            // it's just a mistake of user programs.
        }
        #endif
        
        // Destroy the thread descriptor.
        // In shared-memory implementation,
        // the descriptor is not locked and always deallocated.
        // In distributed-memory implementation,
        // the descriptor is locked and checked whether the write back is ongoing or not.
        // TODO: Can we merge these two behaviors?
        self_2.on_join_resume(mefdn::move(child_th_2));
        
        // The reference on the stack must be invalid.
        MEFDN_ASSERT(!d.child_th.is_valid());
    }
    
private:
    static transfer_type yield_handler(
        derived_type&   self
    ,   ult_ref_type    prev_th
    ,   void* const     /*unused*/ // TODO
    ) {
        // Call the hook.
        self.template on_after_switch<false>(prev_th, self.get_current_ult());
        
        // Push the parent thread to the bottom.
        // This behavior is still controversial.
        // Practically, it is better than push_top
        // because it avoids deadlocking caused by problematic user programs.
        self.push_bottom( mefdn::move(prev_th) );
        
        // Switch to the resumed context of the following thread.
        // No parameters are passed to the resumed context.
        return { &self };
    }
    
public:
    void yield()
    {
        auto& self = this->derived();
        
        auto th = this->pop_top();
        
        // Suspend the current thread.
        self.template suspend_to_cont<void, &basic_worker::yield_handler, false>(
            mefdn::move(th)
        ,   nullptr
        );
    }
    
private:
    MEFDN_NORETURN
    static void fork_child_first_handler(
        derived_type&   self
    ,   ult_ref_type    parent_th
    ) {
        // Call the hook.
        // A new thread is always not locked.
        self.template on_after_switch<false>(parent_th, self.get_current_ult());
        
        // Push the parent thread to the top.
        // The current thread must be on a different stack from the parent's one
        // before calling this
        // because the parent thread might be stolen and started from another worker.
        self.push_top( mefdn::move(parent_th) );
        
        // The parent's call stack is no longer accessible from the current context
        // because the parent thread might have already started on another worker.
        
        MEFDN_LOG_INFO(
            "msg:Forked a new thread in a child-first manner.\t"
            "{}"
        ,   self.show_ult_ref(self.get_current_ult())
        );
        
        // Copy the ID to the current stack.
        const auto child_id = self.get_current_ult_id();
        
        // Get the reference to the suspension data.
        auto& sus_data = self.get_suspension_data(self.get_current_ult());
        
        // Execute the user-defined function.
        sus_data.func(sus_data.ptr);
        
        // Renew a worker to the child continuation's one.
        auto& self_2 = derived_type::renew_worker(child_id);
        
        // Check that this worker is running the child's continuation.
        self_2.check_current_ult_id(child_id);
        
        // Exit this thread.
        self_2.exit();
        
        // Be careful that the destructors are not called in this function.
    }
    
public:
    void fork_child_first(const allocated_ult& child, const fork_func_type func)
    {
        auto& self = this->derived();
        
        // Suspend the current thread.
        self.template suspend_to_new<&basic_worker::fork_child_first_handler>(child, func);
    }
    
public:
    void detach(const ult_id_type& id)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        auto th = self.get_ult_ref_from_id(id);
        
        bool is_destroyable;
        {
            auto lc = th.get_lock();
            
            if (! th.is_finished(lc))
            {
                // The thread is still being executed.
                
                // Change the thread's state to "detached".
                th.set_detached(lc);
                
                // lc is automatically unlocked when this function finishes.
                return;
            }
            
            // If the thread is still writing back,
            // it cannot be destroyed now.
            is_destroyable = th.is_latest_stamp(lc);
            
            if (! is_destroyable) {
                // If the thread cannot be destroyed now (due to the write back),
                // we need to set the detached flag.
                th.set_detached(lc);
            }
        }
        
        if (is_destroyable) {
            // Free the thread descriptor.
            // Both the thread and the write barrier for it have finished.
            //
            // Both shared-memory and distributed-memory versions destroy the child thread
            // because the write back is already completed.
            self.deallocate_ult( mefdn::move(th) );
        }
    }
       
private:
    static transfer_type exit_handler(
        derived_type&   self
    ,   ult_ref_type    prev_th
    ) {
        bool is_destroyable;
        {
            // This worker has already locked the thread.
            auto lc = prev_th.get_lock(mefdn::adopt_lock);
            
            // Because this thread is detached and written back,
            // no other threads will join and manage its resource.
            is_destroyable = prev_th.is_detached(lc) && prev_th.is_latest_stamp(lc);
            
            // Call the hook.
            self.template on_after_switch<true>(prev_th, self.get_current_ult());
            
            // Unlock the current thread here to free the thread descriptor.
        }
        
        if (is_destroyable) {
            // Free the thread descriptor by itself.
            // In distributed-memory implementation,
            // deallocation is delayed until async_write_barrier (called in on_after_switch) is done.
            self.deallocate_ult( mefdn::move(prev_th) );
        }
        
        MEFDN_LOG_INFO(
            "msg:Exiting this thread.\t"
            "sp:{:x}"
        ,   reinterpret_cast<mefdn::uintptr_t>(MEFDN_GET_STACK_POINTER())
        );
        
        // Switch to the resumed context of the following thread.
        return { &self };
    }
    
public:
    MEFDN_NORETURN
    void exit()
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        ult_ref_type next_th;
        
        {
            auto& current_th = self.get_current_ult();
            
            // Lock this thread; a joiner thread may modify this thread.
            auto lc = current_th.get_lock();
            
            // Change the state of this thread.
            current_th.set_finished(lc);
            
            if (current_th.has_joiner(lc)) {
                // Set the joiner thread as the next thread.
                // Note that "get_joiner" may return either ult_ref_type or ult_id_type.
                next_th =
                    self.get_ult_ref(
                        current_th.get_joiner(lc)
                    );
                
                // Change the state of the joiner thread to "ready".
                // This is disabled because it seems unnecessary.
                //next_th.set_ready();
                
                // Call the hook to issue write/read barriers.
                // The joiner thread may have modified data.
                self.on_exit_resume(next_th);
                
                MEFDN_LOG_INFO(
                    "msg:Exiting this thread and switching to the joiner thread.\t"
                    "{}"
                ,   self.show_ult_ref(next_th)
                );
            }
            else {
                // Get the next thread. It might be a root thread.
                next_th = self.pop_top();
                
                MEFDN_LOG_INFO(
                    "msg:Exiting this thread and switching to an unrelated thread.\t"
                    "{}"
                ,   self.show_ult_ref(next_th)
                );
            }
            
            // Release the lock; this lock is unlocked by the handler.
            lc.release();
        }
        
        // Exit the current thread.
        self.template exit_to_cont<&basic_worker::exit_handler, true>(
            mefdn::move(next_th)
        );
        
        /*>--- this context is abandoned ---<*/
        
        // Be careful that the destructors are not called in this function.
        
        MEFDN_UNREACHABLE();
    }
    
    
    ult_ref_type try_steal()
    {
        // "try_pop_bottom" may return either ult_ref_type or ult_id_type.
        auto th = try_pop_bottom();
        
        return th;
    }
    
private:
    void push_top(ult_ref_type&& th)
    {
        MEFDN_LOG_INFO(
            "msg:Push a thread on top.\t"
            "{}"
        ,   derived().show_ult_ref(th)
        );
        
        wd_.push_top(mefdn::move(th));
    }
    void push_bottom(ult_ref_type&& th)
    {
        MEFDN_LOG_INFO(
            "msg:Push a thread on bottom.\t"
            "{}"
        ,   derived().show_ult_ref(th)
        );
        
        wd_.push_bottom(mefdn::move(th));
    }
    
    ult_ref_type pop_top()
    {
        auto th = this->try_pop_top();
        
        if (th.is_valid())
        {
            // Switch to the parent thread.
            
            MEFDN_LOG_INFO(
                "msg:Pop a thread.\t"
                "{}"
            ,   derived().show_ult_ref(th)
            );
        }
        else {
            // There is no parent thread; switch to the root thread.
            
            MEFDN_LOG_INFO(
                "msg:Pop a root thread.\t"
                "{}"
            ,   derived().show_ult_ref(root_th_)
            );
            
            using mefdn::swap;
            swap(th, root_th_);
        }
        
        return th;
    }
    ult_ref_type try_pop_top()
    {
        // "try_pop_top" may return either ult_ref_type or ult_id_type.
        return this->get_ult_ref(
            wd_.try_pop_top()
        );
    }
    ult_ref_type try_pop_bottom()
    {
        // "try_pop_bottom" may return either ult_ref_type or ult_id_type.
        return this->get_ult_ref(
            wd_.try_pop_bottom()
        );
    }
    
    // Overloaded functions to get the descriptor's reference.
    ult_ref_type get_ult_ref(ult_ref_type ref) {
        // Return the identical reference.
        #ifdef MEFDN_CXX11_RETURN_MOVE_ONLY_ARGUMENT_SUPPORTED
            return ref;
        #else
            // This move is only necessary for old GCC
            return mefdn::move(ref);
        #endif
    }
    ult_ref_type get_ult_ref(const ult_id_type& id) {
        // Convert to the reference.
        if (is_invalid_ult_id(id))
            return {};
        else
            return this->derived().get_ult_ref_from_id(id);
    }
    
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
    
    worker_deque_type   wd_;
    
    ult_ref_type        root_th_;
};

} // namespace meult
} // namespace menps

