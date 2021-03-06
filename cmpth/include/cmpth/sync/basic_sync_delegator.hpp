
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_sync_delegator
{
    using sync_queue_type = typename P::sync_queue_type;
    using start_lock_result_type = typename sync_queue_type::start_lock_result;
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using thread_type = typename base_ult_itf_type::thread;
    using suspended_thread_type = typename base_ult_itf_type::suspended_thread;
    using worker_type = typename base_ult_itf_type::worker;

    using sync_node_type = typename P::sync_node_type;

public:
    using consumer_type = typename P::consumer_type;
    using delegated_func_type = typename consumer_type::delegated_func_type;
    
    template <typename DelegateFunc>
    CMPTH_NODISCARD
    bool lock_or_delegate(DelegateFunc&& delegate_func)
    {
        auto lock_ret = this->queue_.start_lock();
        if (CMPTH_LIKELY(lock_ret.is_locked)) {
            return true;
        }
        
        const auto cur = lock_ret.cur;
        suspended_thread_type* const wait_sth =
            fdn::forward<DelegateFunc>(delegate_func)(cur->func);
        
        if (wait_sth) {
            wait_sth->template wait_with<on_wait_delegate>(this, &lock_ret);
        }
        else {
            this->queue_.set_next(lock_ret);
        }
        
        return false;
    }
private:
    struct on_wait_delegate {
        CMPTH_NODISCARD
        bool operator() (
            worker_type&                    /*wk*/
        ,   basic_sync_delegator* const     self
        ,   start_lock_result_type* const   lock_ret
        ) const {
            self->queue_.set_next(*lock_ret);
            return true;
        }
    };
    
public:
    void lock()
    {
        CMPTH_P_LOG_VERBOSE(P, "Start locking delegator.");
        
        const auto ret CMPTH_MAYBE_UNUSED =
            this->lock_or_delegate(on_lock_delegate());
    }
    
private:
    struct on_lock_delegate {
        suspended_thread_type* operator() (delegated_func_type& func) const {
            // TODO: This works, but ugly
            auto* const node = fdn::get_container_of(&func, &sync_node_type::func);
            return &node->sth;
        }
    };
    
public:
    void unlock()
    {
        // We completed the execution of the current critical section.
        this->is_executed_ = true;
        
        CMPTH_P_ASSERT(P, this->con_ != nullptr);
        const bool is_active = this->con_->is_active();
        
        CMPTH_P_LOG_VERBOSE(P,
            "Start unlocking delegator."
        ,   "is_active", is_active
        );
        
        const auto head = this->queue_.get_head();
        
        if (!is_active) {
            if (this->queue_.try_unlock(head)) {
                CMPTH_P_LOG_VERBOSE(P, "Finished unlocking delegator immediately.");
                return;
            }
        }
        
        // If the next acquirer already wrote its pointer,
        // this thread tries to awake its thread directly.
        if (const auto next_head = this->queue_.try_follow_head(head)) {
            //pool.deallocate(head);
            // Note: head is no longer accessible.
            
            // The next critical section is not executed yet.
            this->is_executed_ = false;
            
            if (auto sth = fdn::move(next_head->sth)) {
                CMPTH_P_LOG_VERBOSE(P,
                    "Finish unlocking delegator by awaking next thread."
                );
                
                if (P::prefer_execute_critical) {
                    // Enter the succeeding next thread immediately.
                    sth.enter();
                }
                else {
                    // Make the next thread executable.
                    sth.notify();
                }
                
                return;
            }
        }
        
        CMPTH_P_LOG_VERBOSE(P,
            "Finish unlocking delegator by awaking helper thread."
        );
        
        // Awake the helper thread.
        if (P::prefer_execute_critical) {
            // Enter the helper thread immediately.
            this->con_sth_.enter();
        }
        else {
            // Make the helper thread executable.
            this->con_sth_.notify();
        }
    }
    
    void unlock_and_wait(suspended_thread_type& wait_sth)
    {
        // We completed the execution of the current critical section.
        this->is_executed_ = true;
        
        CMPTH_P_ASSERT(P, this->con_ != nullptr);
        const bool is_active = this->con_->is_active();
        
        CMPTH_P_LOG_VERBOSE(P
        ,   "Start unlocking delegator."
        ,   "is_active", is_active
        );
        
        const auto head = this->queue_.get_head();
        
        if (!is_active) {
            if (this->try_unlock_and_wait(wait_sth, head)) {
                CMPTH_P_LOG_VERBOSE(P,
                    "Finished unlocking delegator immediately.");
                return;
            }
        }
        
        // If the next acquirer already wrote its pointer,
        // this thread tries to awake its thread directly.
        if (const auto next_head = this->queue_.try_follow_head(head)) {
            // Note: head is no longer accessible.
            
            // The next critical section is not executed yet.
            this->is_executed_ = false;
            
            if (auto sth = fdn::move(next_head->sth)) {
                CMPTH_P_LOG_VERBOSE(P,
                    "Finish unlocking delegator by swapping with next thread.");
                
                // Enter the next thread immediately.
                wait_sth.swap(sth);
                
                return;
            }
        }
        
        CMPTH_P_LOG_VERBOSE(P,
            "Finished unlocking delegator by swapping with helper thread.");
        
        // Awake the helper thread.
        // Prefer executing the succeeding critical sections now.
        wait_sth.swap(this->con_sth_);
    }
    
private:
    struct try_unlock_functor
    {
        CMPTH_NODISCARD
        bool operator() (
            worker_type&                /*wk*/
        ,   basic_sync_delegator* const self
        ,   sync_node_type* const       head
        ,   bool* const                 is_unlocked
        ) {
            // Try to unlock the mutex to suspend.
            if (self->queue_.try_unlock(head)) {
                // Important: This thread already released the lock here.
                return true;
            }
            else {
                CMPTH_P_LOG_VERBOSE(P
                ,   "Failed to unlock MCS delegator in try_unlock_functor."
                ,   "head", head
                );
                // Reset this flag.
                *is_unlocked = false;
                return false;
            }
        }
    };
    
    bool try_unlock_and_wait(
        suspended_thread_type&  wait_sth
    ,   sync_node_type* const   head
    ) {
        if (!this->queue_.is_unlockable(head)) {
            return false;
        }
        
        bool is_unlocked = true;
        wait_sth.template wait_with<try_unlock_functor>(this, head, &is_unlocked);
        
        return is_unlocked;
    }
    
public:
    template <typename ImmExecFunc, typename DelegateFunc>
    bool execute_or_delegate(
        ImmExecFunc&&   imm_exec_func
    ,   DelegateFunc&&  delegate_func
    ) {
        const auto is_locked =
            this->lock_or_delegate(
                fdn::forward<DelegateFunc>(delegate_func)
            );
        
        if (is_locked) {
            auto ret = fdn::forward<ImmExecFunc>(imm_exec_func)();
            
            using fdn::get;
            this->is_executed_ = get<0>(ret);
            
            if (suspended_thread_type* const wait_sth = get<1>(ret)) {
                this->unlock_and_wait(*wait_sth);
            }
            else {
                this->unlock();
            }
        }
        
        return false;
    }
    
public:
    void start_consumer(consumer_type& con)
    {
        this->lock();

        this->con_ = &con;
        this->con_th_ = thread_type{ consumer_main{ *this } };
    }
    
    void stop_consumer()
    {
        this->lock();
        
        this->finished_ = true;
        
        const bool is_active = this->con_->is_active();
        
        this->unlock();
        
        if (!is_active) {
            // TODO: Is this correct?
            this->con_sth_.notify();
        }
        
        this->con_th_.join();
    }
    
private:
    struct consumer_main
    {
        basic_sync_delegator&   self;
        
        void operator() ()
        {
            while (!this->self.finished_) {
                this->self.consume();
            }
        }
    };
    
    void consume()
    {
        CMPTH_P_ASSERT(P, this->con_ != nullptr);
        auto& con = *this->con_;
        bool is_executed = this->is_executed_;
        
        CMPTH_P_LOG_VERBOSE(P,
            "Start consuming delegations."
        ,   "is_executed", is_executed
        );
        
        auto head = this->queue_.get_head();
        
        if (is_executed) {
            // Check if there are remaining delegated critical sections.
            if (const auto next_head = this->queue_.try_follow_head(head)) {
                // The next critical section is not executed yet.
                is_executed = false;
                
                // Treat the next critical section as the current one.
                head = next_head;
            }
        }
        
        // True if the progress function is executed.
        bool do_progress = con.is_active();
        
        suspended_thread_type awake_sth;
        
        // Check whether the "head" entry is not executed.
        if (!is_executed) {
            // Check whether the current entry means a lock delegation or a lock transfer.
            if (!head->sth) {
                // Execute the delegated function again.
                auto ret = con.execute(head->func);
                
                using fdn::get;
                is_executed = get<0>(ret);
                awake_sth   = fdn::move(get<1>(ret));

                // TODO: for performance
                if (is_executed) {
                    this->is_executed_ = true;
                    return;
                }
                
                // If the delegation function fails,
                // the progress is required to complete the function again.
                do_progress = !is_executed;
            }
            else {
                CMPTH_P_LOG_VERBOSE(P, "Awake next thread trying to lock delegator.");
                
                // The next thread is trying to lock the mutex.
                this->con_sth_.swap(head->sth);
                
                return;
            }
        }
        
        if (do_progress) {
            // Call the progress function.
            suspended_thread_type ret_awake_sth = con.progress();
            
            if (ret_awake_sth) {
                if (awake_sth) {
                    awake_sth.notify();
                }
                awake_sth = fdn::move(ret_awake_sth);
            }
        }
        
        const bool is_active = con.is_active();

        CMPTH_P_LOG_VERBOSE(P,
            "Finish consuming delegations."
        ,   "is_executed", is_executed
        ,   "is_active", is_active
        ,   "do_progress", do_progress
        );
        
        // Store the flags back to the class members.
        this->is_executed_ = is_executed;
        
        if (!is_active && is_executed) {
            if (awake_sth) {
                if (this->queue_.is_unlockable(head)) {
                    CMPTH_P_LOG_VERBOSE(P,
                        "Try to unlock delegator thread on top of another continuation."
                    ,   "head", head
                    );
                    // The progress is disabled.
                    // Try to unlock the mutex to suspend.
                    bool is_unlocked = true;
                    this->con_sth_.template swap_with<try_unlock_functor>(
                        awake_sth, this, head, &is_unlocked
                    );
                    
                    if (is_unlocked) {
                        // This thread unlocked the mutex and was resumed again.
                        // TODO: This branch is no longer needed.
                    }
                }
            }
            else {
                CMPTH_P_LOG_VERBOSE(P,
                    "Try to unlock delegator thread on top of scheduler context."
                ,   "head", head
                );
                this->try_unlock_and_wait(this->con_sth_, head);
                // Note: Ignore this result.
            }
        }
        
        if (awake_sth) {
            awake_sth.notify();
        }
    }
    
    sync_queue_type     queue_;
    bool                is_executed_ = true;
    bool                is_active_ = false;
    
    consumer_type*          con_ = nullptr;
    thread_type             con_th_;
    suspended_thread_type   con_sth_;
    bool                    finished_ = false;
};

} // namespace cmpth

