
#pragma once

#include <mgbase/utility.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/memory/align.hpp>
#include <mgbase/memory/distance_in_bytes.hpp>

namespace mgult {

template <typename Traits>
class basic_worker
{
private:
    typedef typename Traits::derived_type           derived_type;
    typedef typename Traits::ult_ref_type           ult_ref_type;
    typedef typename Traits::worker_deque_type      worker_deque_type;
    typedef typename Traits::worker_deque_conf_type worker_deque_conf_type;
    typedef typename Traits::ult_id_type            ult_id_type;
    
    typedef void (*fork_func_type)(void*);
    
    template <typename B, typename A>
    struct context
        : Traits::template context<B, A> { };
    
    template <typename B, typename A>
    struct context_result
        : Traits::template context_result<B, A> { };
    
    template <typename B, typename A>
    struct context_argument
        : Traits::template context_argument<B, A> { };
    
    typedef typename context<derived_type, derived_type>::type  ult_context_type;
    
    struct loop_root_data {
        derived_type&   self;
        ult_ref_type    th;
    };
    
    static typename context_result<derived_type, derived_type>::type
    loop_root_handler(typename context_argument<derived_type, loop_root_data>::type arg)
    MGBASE_NOEXCEPT
    {
        const auto d = arg.data;
        
        auto& self = d->self;
        self.check_current_worker();
        
        // Call the hook.
        self.on_after_switch(self.root_th_, d->th);
        
        // Set the root context.
        self.root_th_.set_context(
            self.template cast_context<derived_type, derived_type>(
                arg.fctx /*>---resuming context---<*/
            )
        );
        
        // Set the executed thread as the current one.
        self.set_current_ult(mgbase::move(d->th));
        
        // Switch to the resumed context of the following thread.
        return { {}, &self };
    }
    
protected:
    explicit basic_worker(const worker_deque_conf_type& conf)
        : wd_(conf)
        { }
    
public:
    void loop()
    {
        auto& self = this->derived();
        
        self.check_current_worker();
        
        while (!self.finished())
        {
            auto th = this->try_pop_top();
            
            if (th.is_valid())
            {
                MGBASE_LOG_INFO(
                    "msg:Popped a thread.\t"
                    "{}"
                ,   derived().show_ult_ref(th)
                );
            }
            else
            {
                th = self.try_steal_from_another();
                
                if (th.is_valid())
                {
                    MGBASE_LOG_INFO(
                        "msg:Stealing succeeded."
                    ,   derived().show_ult_ref(th)
                    );
                }
                else
                {
                    MGBASE_LOG_DEBUG("msg:Stealing failed.");
                    continue;
                }
            }
            
            MGBASE_ASSERT(th.is_valid());
            
            // TODO: reuse the thread descriptor
            this->root_th_ = self.allocate_ult();
            
            const auto root_id = this->root_th_.get_id();
            
            loop_root_data d{ self, mgbase::move(th) };
            
            const auto ctx = d.th.get_context();
            
            MGBASE_LOG_INFO(
                "msg:Set up a root thread. Resume the thread.\t"
                "{}"
            ,   self.show_ult_ref(this->root_th_)
            );
            
            // Call the hook.
            self.on_before_switch(this->root_th_, d.th);
            
            // Switch to the context of the next thread.
            auto r = self.ontop_context(ctx, &d, &loop_root_handler);
            
            /*>---resuming context---<*/
            
            MGBASE_ASSERT(&self == r.data);
            self.check_current_worker();
            self.check_current_ult_id(root_id);
            MGBASE_ASSERT(!this->root_th_.is_valid());
            
            MGBASE_LOG_INFO(
                "msg:The thread finished. Came back to the main loop."
            );
            
            auto root_th = this->remove_current_ult();
            
            this->derived().deallocate_ult( mgbase::move(root_th) );
        }
    }
    
private:
    struct fork_child_first_data
    {
        fork_func_type  func;
        void*           ptr;
        
        derived_type&   self;
        ult_ref_type    parent_th;
        ult_ref_type    child_th;
    };
    
    struct fork_parent_first_data
    {
        fork_func_type  func;
        void*           ptr;
        
        ult_id_type     child_id;
    };
    
    union fork_data
    {
        fork_child_first_data   child_first;
        fork_parent_first_data  parent_first;
    };
    
public:
    struct allocated_ult
    {
        ult_id_type id;
        void*       ptr;
    };
    
    MGBASE_WARN_UNUSED_RESULT
    allocated_ult allocate(
        const mgbase::size_t    alignment
    ,   const mgbase::size_t    size
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
        mgbase::size_t stack_size = th.get_stack_size();
        
        // Allocate a space for fork data.
        mgbase::align_call_stack(
            MGBASE_ALIGNOF(fork_data)
        ,   sizeof(fork_data)
        ,   stack_ptr
        ,   stack_size
        );
        
        if (MGBASE_LIKELY(size > 0))
        {
            // Allocate a space for user-defined data.
            const auto ptr =
                mgbase::align_call_stack(
                    alignment
                ,   size
                ,   stack_ptr
                ,   stack_size
                );
            
            if (MGBASE_UNLIKELY(ptr == MGBASE_NULLPTR))
            {
                // The required size by the user is too big
                // to place on the call stack.
                throw std::bad_alloc{};
            }
        }
        
        return { id, stack_ptr };
    }
    
private:
    struct fork_stack_info
    {
        ult_ref_type    child_th;
        void*           stack_ptr;
        mgbase::size_t  stack_size;
        fork_data&      data;
    };
    
    static fork_data& get_fork_data(void* stack_ptr, mgbase::size_t stack_size)
    {
        auto p =
            static_cast<fork_data*>(
                mgbase::align_call_stack(
                    MGBASE_ALIGNOF(fork_data)
                ,   sizeof(fork_data)
                ,   stack_ptr
                ,   stack_size
                )
            );
        
        return *p;
    }
    
    fork_stack_info calc_fork_stack_info(const allocated_ult& child)
    {
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
            static_cast<mgbase::size_t>(
                mgbase::distance_in_bytes(stack_ptr, orig_stack_ptr)
                * MGBASE_CALL_STACK_GROW_DIR
            );
        
        return {
            mgbase::move(child_th)
        ,   stack_ptr
        ,   orig_stack_size - used_size
        ,   get_fork_data(orig_stack_ptr, orig_stack_size)
        };
    }
    
private:
    MGBASE_NORETURN
    static void fork_child_first_handler
    (const typename context_argument<derived_type, fork_child_first_data>::type arg)
    MGBASE_NOEXCEPT
    {
        auto& d = *arg.data;
        
        auto& self = d.self;
        self.check_current_worker();
        
        // Call the hook.
        self.on_after_switch(d.parent_th, d.child_th);
        
        {
            auto& parent_th = d.parent_th;
            
            // Set the context to the parent thread.
            parent_th.set_context(
                self.template cast_context<derived_type, derived_type>(
                    arg.fctx /*>---resuming context---<*/
                )
            );
            
            // Push the parent thread to the top.
            // The current thread must be on a different stack from the parent's one
            // before calling this
            // because the parent thread might be stolen and started from another worker.
            self.push_top( mgbase::move(parent_th) );
        }
        
        // The parent's call stack is no longer accessible from the current context
        // because the parent thread might have already started on another worker.
        
        MGBASE_LOG_INFO(
            "msg:Forked a new thread in a child-first manner.\t"
            "{}"
        ,   self.show_ult_ref(d.child_th)
        );
        
        // Copy the ID to the current stack.
        const auto child_id = d.child_th.get_id();
        
        // Change this thread to the child thread.
        self.set_current_ult(mgbase::move(d.child_th));
        
        // Execute the user-defined function.
        d.func(d.ptr);
        
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
        self.check_current_worker();
        
        auto info = self.calc_fork_stack_info(child);
        
        // Place the thread data on the child's stack.
        auto& d =
            * new (&info.data.child_first) fork_child_first_data{
                func
            ,   child.ptr
            ,   self
            ,   self.remove_current_ult()
            ,   mgbase::move(info.child_th)
            };
        
        // Call the hook.
        self.on_before_switch(d.parent_th, d.child_th);
        
        // Switch to the child's stack
        // and save the context for the parent thread.
        auto r = self.template jump_new_context<derived_type>(
            info.stack_ptr
        ,   info.stack_size
        ,   &fork_child_first_handler
        ,   &d
        );
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        // Renew the worker.
        auto& self_2 = *r.data;
        self_2.check_current_worker();
        
        MGBASE_ASSERT(self_2.current_th_.is_valid());
        
        MGBASE_LOG_INFO(
            "msg:Parent thread forked in a child-first manner was resumed.\t"
            "{}"
        ,   self_2.show_ult_ref(self_2.current_th_)
        );
    }
    
private:
    MGBASE_NORETURN
    static void fork_parent_first_handler
    (const typename context_argument<derived_type, derived_type>::type arg)
    MGBASE_NOEXCEPT
    {
        // Get the worker reference passed by the previous thread.
        auto& self = *arg.data;
        
        const auto orig_stack_ptr = self.current_th_.get_stack_ptr();
        const auto orig_stack_size = self.current_th_.get_stack_size();
        
        // Get the reference to the fork data.
        auto& d = get_fork_data(orig_stack_ptr, orig_stack_size).parent_first;
        
        const auto child_id = d.child_id;
        
        self.check_current_worker();
        self.check_current_ult_id(child_id);
        
        MGBASE_LOG_INFO(
            "msg:Child thread forked in a parent-first manner was resumed.\t"
            "{}"
        ,   self.show_ult_ref(self.current_th_)
        );
        
        // Execute the user-specified function on this stack.
        d.func(d.ptr);
        
        // Renew the worker again.
        auto& self_2 = derived_type::renew_worker(child_id);
        self_2.check_current_ult_id(child_id);
        
        // Destruct the data.
        d.~fork_parent_first_data();
        
        // Exit this thread.
        self_2.exit();
        
        // Be careful that the destructors are not called in this function.
    }
    
public:
    void fork_parent_first(const allocated_ult& child, const fork_func_type func)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        auto info = self.calc_fork_stack_info(child);
        
        // Place the thread data on the child's stack.
        auto& d =
            * new (&info.data.parent_first) fork_parent_first_data{
                func
            ,   child.ptr
            ,   child.id
            };
        
        // Prepare a context.
        const auto ctx =
            self.make_context(
                info.stack_ptr // The call stack starts from here.
            ,   info.stack_size
            ,   &fork_parent_first_handler
            );
        
        // Set the context.
        info.child_th.set_context(ctx);
        
        MGBASE_LOG_INFO(
            "msg:Parent thread forked in a parent-first manner is pushed.\t"
            "data_ptr:{:x}\t"
            "{}"
        ,   reinterpret_cast<mgbase::uintptr_t>(&d)
        ,   self.show_ult_ref(info.child_th)
        );
        
        // Push the child thread on the top.
        self.push_top( mgbase::move(info.child_th) );
    }
    
private:
    struct join_data
    {
        derived_type&   self;
        ult_id_type     child_id;
        
        ult_ref_type    this_th;
        ult_ref_type    child_th;
        ult_ref_type    next_th;
    };
    
    static typename context_result<derived_type, derived_type>::type
    join_handler(const typename context_argument<derived_type, join_data>::type arg)
    MGBASE_NOEXCEPT
    {
        const auto d = arg.data;
        
        auto& self = d->self;
        
        // Move the arguments to the child's stack.
        auto next_th = mgbase::move(d->next_th);
        
        // Call the hook.
        self.on_after_switch(d->this_th, next_th);
        
        {
            // Set this thread as blocked.
            d->this_th.set_blocked();
            
            // Assign the previous context.
            d->this_th.set_context(
                self.template cast_context<derived_type, derived_type>(
                    arg.fctx /*>---resuming context---<*/
                )
            );
            
            // This worker already has a lock of the child thread.
            auto lc = d->child_th.get_lock(mgbase::adopt_lock);
            
            MGBASE_LOG_INFO(
                "msg:Set the current thread as the joiner thread.\t"
                "{}"
            ,   self.show_ult_ref(d->this_th)
            );
            
            d->child_th.set_joiner( mgbase::move(d->this_th) );
            
            // The child thread is automatically unlocked here.
        }
        
        // Set the following thread as the current thread.
        self.set_current_ult( mgbase::move(next_th) );
        
        // Switch to the resumed context of the following thread.
        return { { }, &self };
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
            MGBASE_ASSERT(!child_th.is_detached());
            
            if (child_th.is_finished()) {
                // The child thread has already finished.
                
                MGBASE_LOG_INFO(
                    "msg:Join a thread that already finished.\t"
                    "{}"
                ,   self.show_ult_ref(child_th)
                );
                
                // Call the hook to issue write/read barriers.
                // The joined thread may have modified data.
                self.on_join_acquire(child_th);
                
                // Unlock the child thread to destroy the thread descriptor.
                lc.unlock();
                
                // Destroy the thread descriptor of the child thread.
                // The thread is not detached because it is being joined,
                self.deallocate_ult( mgbase::move(child_th) );
                
                return;
            }
            else {
                MGBASE_LOG_INFO(
                    "msg:Joining a thread that is still running.\t"
                    "{}"
                ,   self.show_ult_ref(child_th)
                );
            }
            
            // Release the lock; this lock is unlocked by the handler.
            lc.release();
        }
        
        join_data d{
            self
        ,   id
        ,   self.remove_current_ult()
        ,   mgbase::move(child_th)
        ,   self.pop_top() // Get the next thread. It might be a root thread.
        };
        
        // Get the context of the thread executed next.
        const auto ctx = d.next_th.get_context();
        
        // Call the hook.
        self.on_before_switch(d.this_th, d.next_th);
        
        // Switch to the context of the next thread.
        auto r = self.ontop_context(ctx, &d, &join_handler);
        
        /*>---resuming context---<*/
        
        // this pointer is no longer available.
        
        auto& self_2 = *r.data;
        self_2.check_current_worker();
        
        MGBASE_LOG_INFO(
            "msg:Resumed the thread blocked to join a child thread that finished now.\t"
            "{}"
        ,   self_2.show_ult_ref(self_2.current_th_)
        );
        
        // Get the reference to the child thread again
        // because this context is resumed on the worker of the child thread.
        
        auto child_th_2 = self_2.get_ult_ref_from_id(id);
        
        // The child thread already finished
        // because this parent thread is joining.
        MGBASE_ASSERT(child_th_2.is_finished());
        
        // No need to lock here
        // because the child thread has already finished
        // and no other threads are joining.
        
        // If the other threads are joining this child thread,
        // it's just a mistake of user programs.
        
        // Destroy the thread descriptor of the child thread.
        self_2.deallocate_ult( mgbase::move(child_th_2) );
    }
    
private:
    struct yield_data
    {
        derived_type&   self;
        ult_ref_type    next_th;
    };
    
    static typename context_result<derived_type, derived_type>::type
    yield_handler(const typename context_argument<derived_type, yield_data>::type arg)
    MGBASE_NOEXCEPT
    {
        const auto d = arg.data;
        
        auto& self = d->self;
        
        // Call the hook.
        self.on_after_switch(self.current_th_, d->next_th);
        
        {
            auto this_th = self.remove_current_ult();
            
            this_th.set_context(
                self.template cast_context<derived_type, derived_type>(
                    arg.fctx /*>---resuming context---<*/
                )
            );
            
            
            // Push the parent thread to the bottom.
            // This behavior is still controversial.
            // Practically, it is better than push_top
            // because it avoids deadlocking caused by problematic user programs.
            self.push_bottom( mgbase::move(this_th) );
        }
        
        // Set the following thread as the current thread.
        self.set_current_ult( mgbase::move(d->next_th) );
        
        // Switch to the resumed context of the following thread.
        // No parameters are passed to the resumed context.
        return { {}, &self };
    }
    
public:
    void yield()
    {
        auto& self = this->derived();
        
        self.check_current_worker();
        
        yield_data d{
            this->derived()
        ,   this->pop_top()
        };
        
        const auto ctx = d.next_th.get_context();
        
        // Call the hook.
        self.on_before_switch(self.current_th_, d.next_th);
        
        // Switch to the context of the next thread.
        self.ontop_context(ctx, &d, &yield_handler);
        
        /*>---resuming context---<*/
    }
    
    void detach(const ult_id_type& id)
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        auto th = self.get_ult_ref_from_id(id);
        
        {
            auto lc = th.get_lock();
            
            if (!th.is_finished())
            {
                // The thread is still being executed.
                
                // Change the thread's state to "detached".
                th.set_detached();
                
                // lc is automatically unlocked when this function finishes.
                return;
            }
        }
        
        // Free the thread descriptor.
        self.deallocate_ult( mgbase::move(th) );
    }
    
private:
    struct exit_data
    {
        derived_type&       self;
        ult_ref_type        this_th;
        ult_ref_type        next_th;
        ult_context_type    next_ctx;
    };
    
    static typename context_result<derived_type, derived_type>::type
    exit_handler(const typename context_argument<derived_type, exit_data>::type arg)
    MGBASE_NOEXCEPT
    {
        const auto d = arg.data;
        auto& self = d->self;
        
        // Call the hook.
        self.on_after_switch(d->this_th, d->next_th);
        
        {
            // Move the previous thread to the current stack
            // in order to destroy the thread descriptor.
            auto this_th = mgbase::move(d->this_th);
            
            bool is_detached;
            {
                // This worker has already locked the thread.
                auto lc = this_th.get_lock(mgbase::adopt_lock);
                
                is_detached = this_th.is_detached();
                
                // Unlock the current thread here to free the thread descriptor.
            }
            
            if (is_detached) {
                // Because this thread is detached,
                // no other threads will join and manage its resource.
                
                // Free the thread descriptor by itself.
                self.deallocate_ult( mgbase::move(this_th) );
            }
        }
        
        MGBASE_LOG_INFO(
            "msg:Exiting this thread.\t"
            "sp:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(MGBASE_GET_STACK_POINTER())
        );
        
        // Set the following thread as the current thread.
        self.set_current_ult( mgbase::move(d->next_th) );
        
        // Explicitly call the destructors
        // because the previous context will be abandoned.
        // These destructors do nothing in an ordinary implementation.
        d->this_th.~ult_ref_type();
        d->next_th.~ult_ref_type();
        d->next_ctx.~ult_context_type();
        
        #if 0
        MGBASE_LOG_INFO(
            "msg:Switching from exiting thread.\t"
            "return0:{:x}\t"
            "frame0:{:x}\t"
            "return1:{:x}\t"
            "frame1:{:x}\t"
            "return2:{:x}\t"
            "frame2:{:x}\t"
            /*"return3:{:x}\t"
            "frame3:{:x}\t"*/
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_return_address(0))
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_frame_address(0))
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_return_address(1))
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_frame_address(1))
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_return_address(2))
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_frame_address(2))
        /*,   reinterpret_cast<mgbase::uintptr_t>(__builtin_return_address(3))
        ,   reinterpret_cast<mgbase::uintptr_t>(__builtin_frame_address(3))*/
        );
        #endif
        
        // Switch to the resumed context of the following thread.
        return { { }, &self };
    }
    
public:
    MGBASE_NORETURN
    void exit()
    {
        auto& self = this->derived();
        self.check_current_worker();
        
        exit_data d{
            self
        ,   self.remove_current_ult()
        ,   {} // invalid thread
        ,   {} // invalid context
        };
        
        {
            // Lock this thread; a joiner thread may modify this thread.
            auto lc = d.this_th.get_lock();
            
            // Change the state of this thread.
            d.this_th.set_finished();
            
            if (d.this_th.has_joiner())
            {
                // "get_joiner" may return either ult_ref_type or ult_id_type.
                auto joiner = 
                    self.get_ult_ref(
                        d.this_th.get_joiner()
                    );
                
                // Set the joiner thread as the next thread.
                d.next_th = mgbase::move(joiner);
            }
            
            // Release the lock; this lock is unlocked by the handler.
            lc.release();
        }
        
        if (d.next_th.is_valid()) {
            // Change the state of the joiner thread to "ready".
            d.next_th.set_ready();
            
            // Call the hook to issue write/read barriers.
            // The joiner thread may have modified data.
            self.on_join_acquire(d.next_th);
            
            MGBASE_LOG_INFO(
                "msg:Exiting this thread and switching to the joiner thread.\t"
                "{}"
            ,   derived().show_ult_ref(d.next_th)
            );
        }
        else {
            // Get the next thread. It might be a root thread.
            d.next_th = self.pop_top();
            
            MGBASE_LOG_INFO(
                "msg:Exiting this thread and switching to an unrelated thread.\t"
                "{}"
            ,   derived().show_ult_ref(d.next_th)
            );
        }
        
        // Get the context of the following thread.
        d.next_ctx = d.next_th.get_context();
        
        // Call the hook.
        self.on_before_switch(d.this_th, d.next_th);
        
        // Switch to the context of the following thread.
        self.ontop_context(d.next_ctx, &d, &exit_handler);
        
        /*>--- this context is abandoned ---<*/
        
        // Be careful that the destructors are not called in this function.
        
        MGBASE_UNREACHABLE();
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
        MGBASE_LOG_INFO(
            "msg:Push a thread on top.\t"
            "{}"
        ,   derived().show_ult_ref(th)
        );
        
        wd_.push_top(mgbase::move(th));
    }
    void push_bottom(ult_ref_type&& th)
    {
        MGBASE_LOG_INFO(
            "msg:Push a thread on bottom.\t"
            "{}"
        ,   derived().show_ult_ref(th)
        );
        
        wd_.push_bottom(mgbase::move(th));
    }
    
    ult_ref_type pop_top()
    {
        auto th = this->try_pop_top();
        
        if (th.is_valid())
        {
            // Switch to the parent thread.
            
            MGBASE_LOG_INFO(
                "msg:Pop a thread.\t"
                "{}"
            ,   derived().show_ult_ref(th)
            );
        }
        else {
            // There is no parent thread; switch to the root thread.
            
            MGBASE_LOG_INFO(
                "msg:Pop a root thread.\t"
                "{}"
            ,   derived().show_ult_ref(root_th_)
            );
            
            using mgbase::swap;
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
    
    void check_current_ult_id(const ult_id_type& id)
    {
        Traits::check_ult_id(current_th_, id);
    }
    
    // Overloaded functions to get the descriptor's reference.
    ult_ref_type get_ult_ref(ult_ref_type ref) {
        // Return the identical reference.
        return ref;
    }
    ult_ref_type get_ult_ref(const ult_id_type& id) {
        // Convert to the reference.
        if (is_invalid_ult_id(id))
            return {};
        else
            return this->derived().get_ult_ref_from_id(id);
    }
    
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    
    worker_deque_type   wd_;
    
    // The current thread can be modified by the current worker
    // without acquiring a lock.
    ult_ref_type        current_th_;
    ult_ref_type        root_th_;
};

} // namespace mgult

