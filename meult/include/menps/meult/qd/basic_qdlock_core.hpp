
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_qdlock_core
{
    using qdlock_node_type = typename P::qdlock_node_type;
    using atomic_node_ptr_type = typename P::atomic_node_ptr_type;
    
public:
    basic_qdlock_core()
        : tail_(nullptr)
        , head_(nullptr)
    { }
    
    template <typename DelegateFunc>
    MEFDN_NODISCARD
    bool lock_or_delegate(
        qdlock_node_type* const cur
    ,   DelegateFunc&&          delegate_func
    ) {
        const auto prev = this->exchange_tail(cur);
        
        if (prev == nullptr) {
            // This thread locked the mutex immediately.
            
            MEFDN_ASSERT(this->head_ == nullptr);
            this->head_ = cur;
            
            MEFDN_LOG_VERBOSE(
                "msg:Locked qdlock immediately.\t"
                "this:{}\t"
                "prev_tail:{}\t"
                "cur_tail:{}"
            ,   mefdn::show_param(this)
            ,   mefdn::show_param(prev)
            ,   mefdn::show_param(cur)
            );
            
            return true;
        }
        else {
            // This thread couldn't lock the mutex immediately.
            mefdn::forward<DelegateFunc>(delegate_func)(*cur);
            
            this->set_next(prev, cur);
            
            MEFDN_LOG_VERBOSE(
                "msg:Failed to lock qdlock immediately.\t"
                "prev:{}\t"
                "cur:{}"
            ,   mefdn::show_param(prev)
            ,   mefdn::show_param(cur)
            );
            
            return false;
        }
    }
    
    template <typename DelegateFunc, typename WaitFunc>
    bool lock_and_wait(
        qdlock_node_type* const cur
    ,   DelegateFunc&&          delegate_func
    ,   WaitFunc&&              wait_func
    ) {
        const auto is_locked =
            this->lock_or_delegate(
                cur
            ,   mefdn::forward<DelegateFunc>(delegate_func)
            );
        
        if (!is_locked) {
            mefdn::forward<WaitFunc>(wait_func)();
        }
        
        return is_locked;
    }
    
    qdlock_node_type* get_head() const noexcept {
        MEFDN_LOG_VERBOSE(
            "msg:Reading qdlock head.\t"
            "head:{}\t"
            "tail:{}"
        ,   mefdn::show_param(this->head_)
        ,   mefdn::show_param(this->tail_.load(mefdn::memory_order_relaxed))
        );
        
        // This thread must be locking this lock.
        MEFDN_ASSERT(this->head_ != nullptr);
        return this->head_;
    }
    
    qdlock_node_type* get_next_head() const noexcept {
        const auto head = this->get_head();
        return head->next.load(mefdn::memory_order_acquire);
    }
    
    MEFDN_NODISCARD
    qdlock_node_type* try_unlock() noexcept
    {
        const auto next_head = this->get_next_head();
        if (MEFDN_UNLIKELY(next_head != nullptr)) {
            MEFDN_LOG_VERBOSE(
                "msg:Failed to unlock qdlock due to succeeding thread.\t"
                "head:{}\t"
                "tail:{}"
            ,   mefdn::show_param(this->head_)
            ,   mefdn::show_param(this->tail_.load(mefdn::memory_order_relaxed))
            );
            return nullptr;
        }
        
        const auto head = this->get_head();
        this->head_ = nullptr;
        if (this->compare_exchange_tail(head)) {
            MEFDN_LOG_VERBOSE(
                "msg:Successfully unlocked qdlock.\t"
                "old_head:{}\t"
                "tail:{}"
            ,   mefdn::show_param(head)
            ,   mefdn::show_param(this->tail_.load(mefdn::memory_order_relaxed))
            );
            return head;
        }
        this->head_ = head;
        
        MEFDN_LOG_VERBOSE(
            "msg:Failed to unlock qdlock due to CAS failure.\t"
            "head:{}\t"
            "tail:{}"
        ,   mefdn::show_param(head)
        ,   mefdn::show_param(this->tail_.load(mefdn::memory_order_relaxed))
        );
        
        return nullptr;
    }
    
    MEFDN_NODISCARD
    qdlock_node_type* try_follow_head() noexcept
    {
        if (const auto next_head = this->get_next_head()) {
            const auto head = this->get_head();
            this->head_ = next_head;
            return head;
        }
        else
            return nullptr;
    }
    
    #if 0
    template <typename YieldFunc>
    MEFDN_NODISCARD
    qdlock_node_type* spin_unlock(YieldFunc yield_func)
    {
        if (const auto old_head = this->try_unlock()) {
            return old_head;
        }
        
        const auto head = this->get_head();
        
        while (true) {
            
            const auto next_head = this->get_next_head();
            if (next_head != nullptr) { return head; }
            
            yield_func();
        }
    }
    #endif
    
private:
    MEFDN_NODISCARD
    qdlock_node_type* exchange_tail(qdlock_node_type* const cur) noexcept
    {
        MEFDN_ASSERT(cur != nullptr);
        
        cur->next.store(nullptr, mefdn::memory_order_relaxed);
        
        const auto prev =
            this->tail_.exchange(cur, mefdn::memory_order_acq_rel);
                // TODO: vs. seq_cst
        
        return prev;
    }
    
    MEFDN_NODISCARD
    bool compare_exchange_tail(qdlock_node_type* const head) noexcept
    {
        auto expected = head;
        if (this->tail_.load(mefdn::memory_order_relaxed) != expected) {
            return false;
        }
        
        return this->tail_.compare_exchange_strong(
            expected, nullptr, mefdn::memory_order_release);
    }
    
    /*static*/ void set_next(
        qdlock_node_type* const prev
    ,   qdlock_node_type* const cur
    ) noexcept
    {
        MEFDN_ASSERT(prev != nullptr);
        MEFDN_ASSERT(cur != nullptr);
        
        MEFDN_ASSERT(prev->next.load() == nullptr);
        
        // Set the next pointer of the previous lock owner.
        prev->next.store(cur, mefdn::memory_order_release);
    }
    
    // The atomic pointer to the latest node.
    atomic_node_ptr_type tail_; // atomic<qdlock_node_type*>
    
    qdlock_node_type* head_;
};

} // namespace meult
} // namespace menps

