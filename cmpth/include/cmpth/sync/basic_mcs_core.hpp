
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_core
{
    using mcs_node_type = typename P::mcs_node_type;
    using atomic_node_ptr_type = typename P::atomic_node_ptr_type;
    
public:
    basic_mcs_core() noexcept
        : tail_{nullptr}
    { }
    
    basic_mcs_core(const basic_mcs_core&) = delete;
    basic_mcs_core& operator = (const basic_mcs_core&) = delete;
    
    CMPTH_NODISCARD
    mcs_node_type* start_lock(mcs_node_type* const cur) noexcept
    {
        CMPTH_P_ASSERT(P, cur != nullptr);
        
        cur->next.store(nullptr, fdn::memory_order_relaxed);
        
        const auto prev =
            this->tail_.exchange(cur, fdn::memory_order_acq_rel);
        
        if (prev == nullptr) {
            // This thread locked the mutex immediately.
            CMPTH_P_ASSERT(P, this->head_ == nullptr);
            this->head_ = cur;
        }
        else {
            // This thread couldn't lock the mutex immediately.
            CMPTH_P_ASSERT(P, prev != cur);
        }
        
        return prev;
    }
    
    static void set_next(
        mcs_node_type* const prev
    ,   mcs_node_type* const cur
    ) noexcept
    {
        CMPTH_P_ASSERT(P, prev != nullptr);
        CMPTH_P_ASSERT(P, cur != nullptr);
        
        CMPTH_P_ASSERT(P, prev->next.load() == nullptr);
        
        // Set the next pointer of the previous lock owner.
        prev->next.store(cur, fdn::memory_order_release);
    }
    
    CMPTH_NODISCARD
    bool try_unlock(mcs_node_type* const head) noexcept
    {
        /*if (this->tail_.load(fdn::memory_order_relaxed) != head) {
            return false;
        }*/
        CMPTH_P_ASSERT(P, this->head_ == head);
        this->head_ = nullptr;
        
        auto expected = head;
        
        const bool ret =
            this->tail_.compare_exchange_strong(
                expected, nullptr, fdn::memory_order_release
            );
        
        if (ret) {
            CMPTH_P_LOG_DEBUG(P,
                "Successfully unlocked MCS core.", 1
            ,   "old_head", head
            );
            return true;
        }
        else {
            CMPTH_P_LOG_DEBUG(P,
                "Failed to unlock MCS core.", 2
            ,   "head", head
            ,   "tail", this->tail_.load(fdn::memory_order_relaxed)
            );
            this->head_ = head;
            return false;
        }
    }
    
    bool is_unlockable(mcs_node_type* const head) const noexcept
    {
        CMPTH_P_ASSERT(P, this->head_ == head);
        return this->tail_.load(fdn::memory_order_relaxed) == head;
    }
    
    mcs_node_type* get_head() const noexcept
    {
        CMPTH_P_LOG_DEBUG(P,
            "Reading head of MCS core.", 2
        ,   "head", this->head_
        ,   "tail", this->tail_.load(fdn::memory_order_relaxed)
        );
        
        // This thread must be locking this lock.
        const auto head = this->head_;
        CMPTH_P_ASSERT(P, head != nullptr);
        return head;
    }
    
    mcs_node_type* try_follow_head(mcs_node_type* const head) noexcept
    {
        CMPTH_P_ASSERT(P, head != nullptr);
        CMPTH_P_ASSERT(P, this->head_ == head);
        
        const auto next = head->next.load(fdn::memory_order_acquire);
        if (next != nullptr) {
            CMPTH_P_ASSERT(P, next != head);
            this->head_ = next;
        }
        return next;
    }
    
private:
    atomic_node_ptr_type tail_;
    
    fdn::byte pad1_[CMPTH_CACHE_LINE_SIZE - sizeof(atomic_node_ptr_type)];
    
    mcs_node_type* head_ = nullptr;
    
    fdn::byte pad2_[CMPTH_CACHE_LINE_SIZE - sizeof(mcs_node_type*)];
};

} // namespace cmpth

