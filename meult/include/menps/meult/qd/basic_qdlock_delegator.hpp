
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/meult/prof.hpp>

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
    template <typename DelegateFunc>
    bool lock_or_delegate(DelegateFunc&& delegate_func)
    {
        auto& self = this->derived();
        auto& pool = self.get_pool();
        
        const auto cur = pool.allocate();
        
        const auto prev = this->core_.start_lock(cur);
        
        if (prev == nullptr) {
            return true;
        }
        else {
            cur->uv = nullptr;
            
            const auto ret =
                mefdn::forward<DelegateFunc>(delegate_func)(*cur);
            
            if (ret.needs_wait) {
                MEFDN_ASSERT(ret.wait_uv != nullptr);
                ret.wait_uv->wait_with(
                    [this, prev, cur] {
                        this->core_.set_next(prev, cur);
                        return true;
                    }
                );
            }
            else {
                this->core_.set_next(prev, cur);
            }
            
            return false;
        }
    }
    
private:
    struct delegate_result {
        bool                    needs_wait;
        uncond_variable_type*   wait_uv;
    };
    
    struct on_lock_wait {
        uncond_variable_type*   uv;
        
        delegate_result operator() (qdlock_node_type& cur) const {
            cur.uv = this->uv;
            return { true, this->uv };
        }
    };
    
public:
    void lock()
    {
        MEFDN_LOG_VERBOSE("msg:Start locking delegator.");
        
        uncond_variable_type uv;
        this->lock_or_delegate(on_lock_wait{ &uv });
        
        // Note: It's OK to deallocate "uv" because .wait() already finished.
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
        
        const auto head = this->core_.get_head();
        auto& consumer_uv = this->th_.get_uv();
        
        if (!this->is_active_) {
            if (this->core_.try_unlock(head)) {
                pool.deallocate(head);
                
                MEFDN_LOG_VERBOSE(
                    "msg:Finished unlocking delegator immediately."
                );
                
                return;
            }
        }
        
        // If the next acquirer already wrote its pointer,
        // this thread tries to awake its thread directly.
        if (const auto next_head = this->core_.get_next_head(head)) {
            const auto uv = next_head->uv;
            if (uv != nullptr) {
                this->core_.follow_head(head, next_head);
                
                pool.deallocate(head);
                
                // Enter the next thread immediately.
                #ifdef MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER
                uv->notify_enter();
                #else
                uv->notify_signal();
                #endif
                
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
        #ifdef MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER
        consumer_uv.notify_enter();
        #else
        consumer_uv.notify_signal();
        #endif
    }
    
    void unlock_and_wait(uncond_variable_type& wait_uv)
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
        
        const auto head = this->core_.get_head();
        auto& consumer_uv = this->th_.get_uv();
        
        if (!this->is_active_) {
            if (this->try_unlock_and_wait(wait_uv, head)) {
                MEFDN_LOG_VERBOSE(
                    "msg:Finished unlocking delegator immediately."
                );
                return;
            }
        }
        
        // If the next acquirer already wrote its pointer,
        // this thread tries to awake its thread directly.
        if (const auto next_head = this->core_.get_next_head(head)) {
            const auto uv = next_head->uv;
            if (uv != nullptr) {
                this->core_.follow_head(head, next_head);
                
                pool.deallocate(head);
                
                // Enter the next thread immediately.
                wait_uv.swap(*uv);
                
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
        wait_uv.swap(consumer_uv);
    }
    
private:
    struct try_unlock_functor
    {
        derived_type&       self;
        qdlock_node_type*   head;
        bool*               is_unlocked;
        
        MEFDN_NODISCARD
        bool operator() () {
            // Try to unlock the mutex to suspend.
            if (this->self.core_.try_unlock(this->head)) {
                // Important: This thread already released the lock here.
                
                auto& pool = this->self.get_pool();
                // Release the resource.
                pool.deallocate(this->head);
                
                return true;
            }
            else {
                // Reset this flag.
                *this->is_unlocked = false;
                
                return false;
            }
        }
    };
    
    bool try_unlock_and_wait(
        uncond_variable_type&   wait_uv
    ,   qdlock_node_type* const head
    ) {
        auto& self = this->derived();
        
        if (!this->core_.is_unlockable(head)) {
            return false;
        }
        
        bool is_unlocked = true;
        wait_uv.wait_with(try_unlock_functor{ self, head, &is_unlocked });
        
        return is_unlocked;
    }
    
public:
    template <typename ImmExecFunc, typename DelegateFunc>
    bool execute_or_delegate(
        ImmExecFunc&&           imm_exec_func
    ,   DelegateFunc&&          delegate_func
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
            
            if (ret.needs_wait) {
                MEFDN_ASSERT(ret.wait_uv != nullptr);
                this->unlock_and_wait(*ret.wait_uv);
            }
            else {
                this->unlock();
            }
        }
        
        return false;
    }
    
public:
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
        this->lock();
        
        this->th_.set_finished();
        
        const bool is_active = this->is_active_;
        
        this->unlock();
        
        if (!is_active) {
            this->th_.get_uv().notify_signal();
        }
        
        this->th_.stop();
    }
    
private:
    template <typename DelExecFunc, typename ProgressFunc>
    void consume(
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
        
        auto head = this->core_.get_head();
        auto& consumer_uv = this->th_.get_uv();
        
        if (is_executed) {
            // Check if there are remaining delegated critical sections.
            if (const auto next_head = this->core_.get_next_head(head)) {
                this->core_.follow_head(head, next_head);
                
                // Free the old entry because we now got the next pointer.
                pool.deallocate(head);
                
                is_executed = false;
                
                head = next_head;
            }
        }
        
        // True if the progress function is executed.
        bool do_progress = is_active;
        
        bool do_switch = false;
        uncond_variable_type* switch_uv = nullptr;
        
        // Check whether the "head" entry is not executed.
        if (!is_executed) {
            // Check whether the current entry means a lock delegation or a lock transfer.
            if (head->uv == nullptr) {
                // Execute the delegated function again.
                const auto ret = del_exec_func(*head);
                
                is_executed = ret.is_executed;
                is_active   = ret.is_active;
                
                if (ret.needs_awake) {
                    do_switch = true;
                    switch_uv = ret.awake_uv;
                }
                
                // If the delegation function fails,
                // the progress is required to complete the function again.
                do_progress = !is_executed;
            }
            else {
                MEFDN_LOG_VERBOSE("msg:Awake next thread trying to lock qdlock.");
                
                // The next thread is trying to lock the mutex.
                consumer_uv.swap(*head->uv);
                
                return;
            }
        }
        
        if (do_progress) {
            prof::add(prof_kind::qd_progress_count, this->core_.get_count());
            
            // Call the progress function.
            const auto ret = progress_func();
            
            is_active = ret.is_active;
            
            if (ret.needs_awake) {
                if (do_switch) {
                    MEFDN_ASSERT(switch_uv != nullptr);
                    switch_uv->notify_signal();
                }
                do_switch = true;
                switch_uv = ret.awake_uv;
            }
        }
        
        MEFDN_LOG_VERBOSE(
            "msg:Finish consuming qdlock delegations.\t"
            "is_executed:{}\t"
            "is_active:{}\t"
            "do_progress:{}\t"
        ,   is_executed
        ,   is_active
        ,   do_progress
        );
        
        // Store the flags back to the class members.
        this->is_executed_ = is_executed;
        this->is_active_ = is_active;
        
        if (!is_active && is_executed) {
            prof::add(prof_kind::qd_consume_count, this->core_.get_count());
            
            if (do_switch) {
                MEFDN_ASSERT(switch_uv != nullptr);
                
                if (this->core_.is_unlockable(head)) {
                    // The progress is disabled.
                    // Try to unlock the mutex to suspend.
                    bool is_unlocked = true;
                    consumer_uv.swap_with(
                        *switch_uv
                    ,   try_unlock_functor{ self, head, &is_unlocked }
                    );
                    
                    if (is_unlocked) {
                        // This thread unlocked the mutex and was resumed again.
                        do_switch = false;
                        
                        prof::add(prof_kind::qd_consume_unlock_succ, 1);
                    }
                    else {
                        prof::add(prof_kind::qd_consume_unlock_fail, 1);
                    }
                }
            }
            else {
                if (this->try_unlock_and_wait(consumer_uv, head)) {
                    prof::add(prof_kind::qd_consume_unlock_succ, 1);
                }
                else {
                    prof::add(prof_kind::qd_consume_unlock_fail, 1);
                }
            }
        }
        
        if (do_switch) {
            MEFDN_ASSERT(switch_uv != nullptr);
            switch_uv->notify_signal();
        }
    }
    
    qdlock_core_type    core_;
    qdlock_thread_type  th_;
    bool                is_executed_ = false;
    bool                is_active_ = false;
};


} // namespace meult
} // namespace menps

