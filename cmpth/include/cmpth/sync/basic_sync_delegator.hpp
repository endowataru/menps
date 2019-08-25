
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

//#define MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER

template <typename P>
class basic_sync_delegator
{
    using sync_queue_type = typename P::sync_queue_type;
    using start_lock_result_type = typename sync_queue_type::start_lock_result;
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using thread_type = typename base_ult_itf_type::thread;
    using suspended_thread_type = typename base_ult_itf_type::suspended_thread;
    using worker_type = typename base_ult_itf_type::worker;
    
public:
    using sync_node_type = typename P::sync_node_type;
    
    template <typename DelegateFunc>
    CMPTH_NODISCARD
    bool lock_or_delegate(/*worker_type& wk, */DelegateFunc&& delegate_func)
    {
        auto lock_ret = this->queue_.start_lock();
        if (CMPTH_LIKELY(lock_ret.is_locked)) {
            return true;
        }
        
        const auto cur = lock_ret.cur;
        const auto del_ret =
            fdn::forward<DelegateFunc>(delegate_func)(*cur);
        
        if (suspended_thread_type* const wait_sth = del_ret.wait_sth) {
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
        CMPTH_P_LOG_DEBUG(P,"Start locking delegator.", 0);
        
        this->lock_or_delegate(on_lock_delegate());
    }
    
private:
    struct delegate_result {
        suspended_thread_type*  wait_sth;
    };
    
    struct on_lock_delegate {
        delegate_result operator() (sync_node_type& cur) const {
            return { &cur.sth };
        }
    };
    
public:
    void unlock()
    {
        // We completed the execution of the current critical section.
        this->is_executed_ = true;
        
        CMPTH_P_LOG_DEBUG(P,
            "Start unlocking delegator.", 1
        ,   "is_active", this->is_active_
        );
        
        const auto head = this->queue_.get_head();
        
        if (!this->is_active_) {
            if (this->queue_.try_unlock(head)) {
                CMPTH_P_LOG_DEBUG(P,
                    "Finished unlocking delegator immediately.", 0
                );
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
                // Enter the next thread immediately.
                #ifdef MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER
                sth.enter();
                #else
                sth.notify();
                #endif
                
                CMPTH_P_LOG_DEBUG(P,
                    "Finished unlocking delegator by awaking next thread.", 0
                );
                
                return;
            }
        }
        
        CMPTH_P_LOG_DEBUG(P,
            "Finished unlocking delegator by awaking helper thread.", 0
        );
        
        // Awake the helper thread.
        #ifdef MEULT_QD_USE_UNCOND_ENTER_FOR_TRANSFER
        // Prefer executing the succeeding critical sections now.
        this->con_sth_.enter();
        #else
        this->con_sth_.notify();
        #endif
    }
    
    void unlock_and_wait(suspended_thread_type& wait_sth)
    {
        // We completed the execution of the current critical section.
        this->is_executed_ = true;
        
        CMPTH_P_LOG_DEBUG(P,
            "Start unlocking delegator.", 1
        ,   "is_active", this->is_active_
        );
        
        const auto head = this->queue_.get_head();
        
        if (!this->is_active_) {
            if (this->try_unlock_and_wait(wait_sth, head)) {
                CMPTH_P_LOG_DEBUG(P,
                    "Finished unlocking delegator immediately.", 0
                );
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
                // Enter the next thread immediately.
                wait_sth.swap(sth);
                
                CMPTH_P_LOG_DEBUG(P,
                    "Finished unlocking delegator by awaking next thread.", 0
                );
                
                return;
            }
        }
        
        CMPTH_P_LOG_DEBUG(P,
            "Finished unlocking delegator by awaking helper thread.", 0
        );
        
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
            
            this->is_executed_ = ret.is_executed;
            this->is_active_   = ret.is_active;
            
            if (suspended_thread_type* const wait_sth = ret.wait_sth) {
                this->unlock_and_wait(*wait_sth);
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
        this->lock();
        
        using del_exec_func_type = fdn::decay_t<DelExecFunc>;
        using progress_func_type = fdn::decay_t<ProgressFunc>;
        
        // Note: Copy the functions to a new thread's call stack.
        this->con_th_ = thread_type{
            consumer_main<del_exec_func_type, progress_func_type>{
                *this
            ,   fdn::forward<DelExecFunc>(del_exec_func)
            ,   fdn::forward<ProgressFunc>(progress_func)
            }
        };
    }
    
    void stop_consumer()
    {
        this->lock();
        
        this->finished_ = true;
        
        const bool is_active = this->is_active_;
        
        this->unlock();
        
        if (!is_active) {
            // TODO: Is this correct?
            this->con_sth_.notify();
        }
        
        this->con_th_.join();
    }
    
private:
    template <typename DelExecFunc, typename ProgressFunc>
    struct consumer_main
    {
        basic_sync_delegator&   self;
        DelExecFunc             del_exec_func;
        ProgressFunc            progress_func;
        
        void operator() ()
        {
            while (!self.finished_) {
                self.consume(del_exec_func, progress_func);
            }
        }
    };
    
    template <typename DelExecFunc, typename ProgressFunc>
    void consume(
        DelExecFunc&    del_exec_func
    ,   ProgressFunc&   progress_func
    ) {
        bool is_executed = this->is_executed_;
        bool is_active   = this->is_active_;
        
        CMPTH_P_LOG_DEBUG(P,
            "Start consuming delegations.\t", 2
        ,   "is_executed", is_executed
        ,   "is_active", is_active
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
        bool do_progress = is_active;
        
        suspended_thread_type awake_sth;
        
        // Check whether the "head" entry is not executed.
        if (!is_executed) {
            // Check whether the current entry means a lock delegation or a lock transfer.
            if (!head->sth) {
                // Execute the delegated function again.
                auto ret = del_exec_func(*head);
                
                is_executed = ret.is_executed;
                is_active   = ret.is_active;
                awake_sth   = fdn::move(ret.awake_sth);
                
                // If the delegation function fails,
                // the progress is required to complete the function again.
                do_progress = !is_executed;
            }
            else {
                CMPTH_P_LOG_DEBUG(P, "Awake next thread trying to lock delegator.", 0);
                
                // The next thread is trying to lock the mutex.
                this->con_sth_.swap(head->sth);
                
                return;
            }
        }
        
        if (do_progress) {
            // Call the progress function.
            auto ret = progress_func();
            
            is_active = ret.is_active;
            
            if (ret.awake_sth) {
                if (awake_sth) {
                    awake_sth.notify();
                }
                awake_sth = fdn::move(ret.awake_sth);
            }
        }
        
        CMPTH_P_LOG_DEBUG(P,
            "Finish consuming delegations.", 3
        ,   "is_executed", is_executed
        ,   "is_active", is_active
        ,   "do_progress", do_progress
        );
        
        // Store the flags back to the class members.
        this->is_executed_ = is_executed;
        this->is_active_ = is_active;
        
        if (!is_active && is_executed) {
            if (awake_sth) {
                if (this->queue_.is_unlockable(head)) {
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
    
    thread_type             con_th_;
    suspended_thread_type   con_sth_;
    bool                    finished_ = false;
};

} // namespace cmpth

