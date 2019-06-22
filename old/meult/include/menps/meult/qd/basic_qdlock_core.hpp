
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
    
    MEFDN_NODISCARD
    qdlock_node_type* start_lock(qdlock_node_type* const cur) noexcept
    {
        MEFDN_ASSERT(cur != nullptr);
        
        cur->next.store(nullptr, mefdn::memory_order_relaxed);
        
        const auto prev =
            this->tail_.exchange(cur, mefdn::memory_order_acq_rel);
                // TODO: vs. seq_cst
        
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
        }
        else {
            // This thread couldn't lock the mutex immediately.
            MEFDN_LOG_VERBOSE(
                "msg:Failed to lock qdlock immediately.\t"
                "prev:{}\t"
                "cur:{}"
            ,   mefdn::show_param(prev)
            ,   mefdn::show_param(cur)
            );
        }
        
        #ifdef MEULT_QD_ENABLE_ATOMIC_COUNT
        this->pro_count_.fetch_add(1, mefdn::memory_order_relaxed);
        #endif
        
        return prev;
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
    
    qdlock_node_type* get_head() const noexcept
    {
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
    
    qdlock_node_type* get_next_head(qdlock_node_type* const head) const noexcept
    {
        MEFDN_ASSERT(this->head_ == head);
        return head->next.load(mefdn::memory_order_acquire);
    }
    
    void follow_head(
        qdlock_node_type* const head
    ,   qdlock_node_type* const next
    ) noexcept
    {
        MEFDN_ASSERT(this->head_ == head);
        MEFDN_ASSERT(head->next.load(mefdn::memory_order_relaxed) == next);
        this->head_ = next;
        
        #ifdef MEULT_QD_ENABLE_ATOMIC_COUNT
        this->con_count_.fetch_add(1, mefdn::memory_order_relaxed);
        #endif
    }
    
    bool is_unlockable(qdlock_node_type* const head) const noexcept
    {
        return this->tail_.load(mefdn::memory_order_relaxed) == head;
    }
    
    MEFDN_NODISCARD
    bool try_unlock(qdlock_node_type* const head) noexcept
    {
        MEFDN_ASSERT(this->head_ == head);
        this->head_ = nullptr;
        
        auto expected = head;
        
        if (this->tail_.load(mefdn::memory_order_relaxed) == expected)
        {
            if (this->tail_.compare_exchange_strong(
                expected, nullptr, mefdn::memory_order_release)
            ) {
                MEFDN_LOG_VERBOSE(
                    "msg:Successfully unlocked qdlock.\t"
                    "old_head:{}\t"
                    "tail:{}"
                ,   mefdn::show_param(head)
                ,   mefdn::show_param(this->tail_.load(mefdn::memory_order_relaxed))
                );
                
                #ifdef MEULT_QD_ENABLE_ATOMIC_COUNT
                this->con_count_.fetch_add(1, mefdn::memory_order_relaxed);
                #endif
                
                return true;
            }
        }
        this->head_ = head;
        
        MEFDN_LOG_VERBOSE(
            "msg:Failed to unlock qdlock due to CAS failure.\t"
            "head:{}\t"
            "tail:{}"
        ,   mefdn::show_param(head)
        ,   mefdn::show_param(this->tail_.load(mefdn::memory_order_relaxed))
        );
        
        return false;
    }
    
    mefdn::size_t get_count() const noexcept
    {
        #ifdef MEULT_QD_ENABLE_ATOMIC_COUNT
        return this->pro_count_.load(mefdn::memory_order_relaxed) -
            this->con_count_.load(mefdn::memory_order_relaxed);
        #else
        return 0;
        #endif
    }
    
private:
    // The atomic pointer to the latest node.
    atomic_node_ptr_type tail_; // atomic<qdlock_node_type*>
    
    mefdn::byte pad1_[MEFDN_CACHE_LINE_SIZE - sizeof(atomic_node_ptr_type)];
    
    qdlock_node_type* head_;
    
    mefdn::byte pad2_[MEFDN_CACHE_LINE_SIZE - sizeof(qdlock_node_type*)];
    
    #ifdef MEULT_QD_ENABLE_ATOMIC_COUNT
    mefdn::atomic<mefdn::size_t> pro_count_;
    
    mefdn::byte pad3_[MEFDN_CACHE_LINE_SIZE - sizeof(mefdn::atomic<mefdn::size_t>)];
    
    mefdn::atomic<mefdn::size_t> con_count_;
    
    mefdn::byte pad4_[MEFDN_CACHE_LINE_SIZE - sizeof(mefdn::size_t)];
    #endif
};

} // namespace meult
} // namespace menps

