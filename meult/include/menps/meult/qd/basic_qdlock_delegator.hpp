
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_qdlock_delegator
{
    MEFDN_DEFINE_DERIVED(P)
    
    using qdlock_core_type = typename P::qdlock_core_type;
    using qdlock_node_type = typename P::qdlock_node_type;
    using qdlock_thread_type = typename P::qdlock_thread_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using uncond_variable_type = typename ult_itf_type::uncond_variable;
    
public:
    void lock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        MEFDN_LOG_VERBOSE("msg:Start locking delegator.");
        
        const auto cur = pool.allocate();
        
        uncond_variable_type uv;
        
        this->core_.lock_and_wait(
            cur
        ,   [&uv] (qdlock_node_type& n) { n.uv = &uv; }
        ,   [&uv] { uv.wait(); }
        );
        
        // Note: It's OK to deallocate "uv" because .wait() already finished.
    }
    
    template <typename DelegateFunc>
    MEFDN_NODISCARD
    bool lock_or_delegate(DelegateFunc&& delegate_func)
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        const auto cur = pool.allocate();
        cur->uv = nullptr;
        
        return this->core_.lock_or_delegate(
            cur
        ,   mefdn::forward<DelegateFunc>(delegate_func)
        );
    }
    
    template <typename ImmExecFunc, typename DelegateFunc>
    bool execute_or_delegate(
        ImmExecFunc&&   imm_exec_func
    ,   DelegateFunc&&  delegate_func
    ) {
        const auto is_locked =
            this->lock_or_delegate(
                mefdn::forward<DelegateFunc>(delegate_func)
            );
        
        if (is_locked) {
            const auto ret =
                mefdn::forward<ImmExecFunc>(imm_exec_func)();
            
            this->is_executed_ = ret.is_executed;
            this->is_active_   = ret.is_active;
            
            this->unlock();
        }
        
        return is_locked;
    }
    
    void unlock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        // We completed the execution of the current critical section.
        this->is_executed_ = true;
        
        MEFDN_LOG_VERBOSE(
            "msg:Start unlocking delegator.\t"
            "is_active:{}"
        ,   this->is_active_
        );
        
        if (!this->is_active_) {
            if (const auto old_head = this->core_.try_unlock()) {
                pool.deallocate(old_head);
                
                MEFDN_LOG_VERBOSE(
                    "msg:Finished unlocking delegator immediately."
                );
                
                return;
            }
        }
        
        // If the next acquirer already wrote its pointer,
        // this thread tries to awake its thread directly.
        if (const auto next_head = this->core_.get_next_head())
        {
            const auto uv = next_head->uv;
            
            if (uv != nullptr)
            {
                const auto old_head = this->core_.try_follow_head();
                MEFDN_ASSERT(old_head != nullptr);
                
                pool.deallocate(old_head);
                
                // Enter the next thread immediately.
                uv->notify_enter();
                
                MEFDN_LOG_VERBOSE(
                    "msg:Finished unlocking delegator by awaking next thread.\t"
                );
                
                return;
            }
        }
        
        MEFDN_LOG_VERBOSE(
            "msg:Finished unlocking delegator by awaking helper thread.\t"
        );
        
        // Awake the helper thread.
        // Prefer executing the succeeding critical sections now.
        this->th_.notify_enter();
    }
    
    template <typename DelExecFunc, typename ProgressFunc>
    void start_consumer(
        DelExecFunc&&   del_exec_func
    ,   ProgressFunc&&  progress_func
    ) {
        // Note: Copy the functions to a new thread's call stack.
        th_.start([this, del_exec_func, progress_func] {
            return this->consume(del_exec_func, progress_func);
        });
    }
    
    void stop_consumer()
    {
        th_.stop();
    }
    
private:
    template <typename DelExecFunc, typename ProgressFunc>
    bool consume(
        DelExecFunc&    del_exec_func
    ,   ProgressFunc&   progress_func
    ) {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        bool is_executed = this->is_executed_;
        bool is_active   = this->is_active_;
        
        MEFDN_LOG_VERBOSE(
            "msg:Start consuming qdlock delegations.\t"
            "is_executed:{}\t"
            "is_active:{}"
        ,   is_executed
        ,   is_active
        );
        
        if (is_executed) {
            // Check if there are remaining delegated critical sections.
            if (const auto old_head = this->core_.try_follow_head()) {
                // Free the old entry because we now got the next pointer.
                pool.deallocate(old_head);
                
                is_executed = false;
            }
        }
        
        // True if the helper thread will sleep after returning this function.
        bool awake = true;
        // True if the progress function is executed.
        bool do_progress = false;
        
        if (is_executed) {
            // The "head" entry was already executed.
            
            if (is_active) {
                // The progress is enabled.
                do_progress = true;
            }
            else {
                // The progress is disabled.
                // Try to unlock the mutex to suspend.
                if (const auto old_head = this->core_.try_unlock()) {
                    // Important: This thread already released the lock here.
                    
                    // Release the resource.
                    pool.deallocate(old_head);
                    
                    // Now this helper thread starts to suspend.
                    awake = false;
                }
            }
        }
        else {
            // Load the head pointer to be executed now.
            const auto head = this->core_.get_head();
            MEFDN_ASSERT(head != nullptr);
            
            // Check whether the current entry means a lock delegation or a lock transfer.
            if (head->uv == nullptr) {
                // Execute the delegated function again.
                const auto ret = del_exec_func(*head);
                
                is_executed = ret.is_executed;
                is_active   = ret.is_active;
                
                // If the delegation function fails,
                // the progress is required to complete the function again.
                do_progress = !is_executed;
            }
            else {
                MEFDN_LOG_VERBOSE("msg:Awake next thread trying to lock qdlock.");
                
                // The next thread is trying to lock the mutex.
                // Prefer entering the next thread immediately.
                head->uv->notify_enter();
                
                // Important: This thread already released the lock here.
                // This helper thread starts to suspend.
                awake = false;
            }
        }
        
        if (awake) {
            if (do_progress) {
                // Call the progress function.
                const auto ret = progress_func();
                
                is_active = ret.is_active;
            }
            
            // Store the flags back to the class members.
            this->is_executed_ = is_executed;
            this->is_active_ = is_active;
        }
        else {
            // Doing the progress while unlocking the mutex is wrong.
            MEFDN_ASSERT(!do_progress);
        }
        
        MEFDN_LOG_VERBOSE(
            "msg:Finish consuming qdlock delegations.\t"
            "is_executed:{}\t"
            "is_active:{}\t"
            "do_progress:{}\t"
            "awake:{}"
        ,   is_executed
        ,   is_active
        ,   do_progress
        ,   awake
        );
        
        return awake;
    }
    
    qdlock_core_type    core_;
    qdlock_thread_type  th_;
    bool                is_executed_ = false;
    bool                is_active_ = false;
};


} // namespace meult
} // namespace menps

