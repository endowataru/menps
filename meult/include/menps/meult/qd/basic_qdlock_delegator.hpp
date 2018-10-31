
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
            this->is_active_ =
                mefdn::forward<ImmExecFunc>(imm_exec_func)();
            
            this->unlock();
        }
        
        return is_locked;
    }
    
    void unlock()
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
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
                
                uv->notify();
                
                MEFDN_LOG_VERBOSE(
                    "msg:Finished unlocking delegator by awaking next thread.\t"
                );
                
                return;
            }
        }
        
        this->th_.notify();
        
        MEFDN_LOG_VERBOSE(
            "msg:Finished unlocking delegator by awaking helper thread.\t"
        );
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
        
        MEFDN_LOG_VERBOSE(
            "msg:Start consuming qdlock delegations."
        );
        
        bool awake = true;
        
        if (const auto old_head = this->core_.try_follow_head())
        {
            // 
            pool.deallocate(old_head);
            
            const auto head = this->core_.get_head();
            
            if (head->uv == nullptr) {
                // Execute the delegated function.
                this->is_active_ = del_exec_func(*head);
            }
            else {
                MEFDN_LOG_VERBOSE("msg:Awake next thread trying to lock qdlock.");
                
                // The next thread is trying to lock the mutex.
                head->uv->notify();
                
                // This helper thread starts to suspend.
                awake = false;
            }
        }
        else {
            if (this->is_active_) {
                this->is_active_ = progress_func();
            }
            
            if (!this->is_active_) {
                // This helper thread needs not to do progress.
                
                // Try to unlock the mutex to suspend.
                if (const auto old_head = this->core_.try_unlock()) {
                    pool.deallocate(old_head);
                    
                    // Now this helper thread starts to suspend.
                    awake = false;
                }
            }
        }
        
        MEFDN_LOG_VERBOSE(
            "msg:Finish consuming qdlock delegations.\t"
            "awake:{}\t"
        ,   awake
        );
        
        return awake;
    }
    
    qdlock_core_type    core_;
    qdlock_thread_type  th_;
    bool                is_active_ = false;
};


} // namespace meult
} // namespace menps

