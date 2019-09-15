
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_ring_buf_core
{
    using base_ult_itf_type = typename P::base_ult_itf_type;

    using ring_buf_count_type = typename P::ring_buf_count_type;
    using atomic_count_type =
        typename base_ult_itf_type::template atomic<ring_buf_count_type>;
    
public:
    struct lock_result {
        bool    is_locked;
        bool    is_delegated;
        ring_buf_count_type prev_tail_idx;
        ring_buf_count_type cur_tail_idx;
    };
    
    CMPTH_NODISCARD
    lock_result try_lock_or_enqueue(const ring_buf_count_type cap)
    {
        const auto head = this->head_.load(fdn::memory_order_acquire);
        auto tail = this->tail_.load(fdn::memory_order_relaxed);
        
        if (tail - head >= ((cap - 1) << 1)) {
            CMPTH_P_LOG_DEBUG(P,
                "Failed to lock full ring buffer core.", 2
            ,   "head", head
            ,   "tail", tail
            );
            return { false, false, 0, 0 };
        }
        
        const bool is_delegated = tail & 1;
        const auto new_tail = tail + (is_delegated ? 2 : 1);
        
        const bool success =
            this->tail_.compare_exchange_weak(
                tail, new_tail
            ,   fdn::memory_order_acquire, fdn::memory_order_relaxed
            );
        
        if (success) {
            const auto prev_tail_idx = (tail >> 1) % cap;
            const auto cur_tail_idx = (new_tail >> 1) % cap;
            
            CMPTH_P_LOG_DEBUG(P,
                "Successfully locked or enqueued to ring buffer core.", 5
            ,   "head", head
            ,   "tail", tail
            ,   "prev_tail_idx", prev_tail_idx
            ,   "cur_tail_idx", cur_tail_idx
            ,   "is_delegated", is_delegated
            );
            return { !is_delegated, is_delegated, prev_tail_idx, cur_tail_idx };
        }
        else {
            CMPTH_P_LOG_DEBUG(P,
                "Failed to exchange ring buffer counter core.", 3
            ,   "head", head
            ,   "tail", tail
            ,   "new_tail", new_tail
            );
            return { false, false, 0, 0 };
        }
    }
    
    CMPTH_NODISCARD
    bool try_unlock()
    {
        const auto head = this->head_.load(fdn::memory_order_relaxed);
        CMPTH_P_ASSERT(P, (head& 1) == 0);
        
        auto tail = head | 1;
        const auto new_tail = head;
        
        const auto success =
            this->tail_.compare_exchange_strong(
                tail, new_tail
            ,   fdn::memory_order_release, fdn::memory_order_relaxed
            );

        if (success) {
            CMPTH_P_LOG_DEBUG(P,
                "Successfully unlocked ring buffer core.", 4
            ,   "head", head
            ,   "tail", tail
            ,   "new_tail", new_tail
            );
        }
        else {
            CMPTH_P_LOG_DEBUG(P,
                "Failed to unlock ring buffer core.", 4
            ,   "head", head
            ,   "tail", tail
            ,   "new_tail", new_tail
            );
        }
        
        return success;
    }
    
    ring_buf_count_type get_head(const ring_buf_count_type cap) const noexcept
    {
        const auto head = this->head_.load(fdn::memory_order_relaxed);
        return (head >> 1) % cap;
    }
    
    void check_head(
        const ring_buf_count_type head_idx
    ,   const ring_buf_count_type cap
    ) const noexcept
    {
        CMPTH_P_ASSERT(P, head_idx ==
            (this->head_.load(fdn::memory_order_relaxed) >> 1) % cap);
    }
    
    ring_buf_count_type follow_head(const ring_buf_count_type cap) noexcept
    {
        const auto head = this->head_.load(fdn::memory_order_relaxed);
        const auto next_head = head + 2;
        
        CMPTH_P_LOG_DEBUG(P,
            "Following head of ring buffer core.", 2
        ,   "head", head
        ,   "next_head", next_head
        );
        
        this->head_.store(next_head, fdn::memory_order_relaxed);
        return (next_head >> 1) % cap;
    }
    
    bool is_unlockable() const noexcept {
        const auto head = this->head_.load(fdn::memory_order_relaxed);
        const auto tail = this->tail_.load(fdn::memory_order_relaxed);
        CMPTH_P_ASSERT(P, (head & 1) == 0);
        CMPTH_P_ASSERT(P, (tail & 1) == 1);
        
        const auto ret = (head | 1) == tail;
        
        CMPTH_P_LOG_DEBUG(P,
            "Checking whether ring buffer is unlockable.", 3
        ,   "head", head
        ,   "tail", tail
        ,   "is_unlockable", ret
        );
        return ret;
    }
    
private:
    atomic_count_type head_;
    
    fdn::byte pad1_[CMPTH_CACHE_LINE_SIZE - sizeof(atomic_count_type)];
    
    atomic_count_type tail_;
    
    fdn::byte pad2_[CMPTH_CACHE_LINE_SIZE - sizeof(atomic_count_type)];
};

} // namespace cmpth

