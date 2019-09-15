
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_ring_buf_queue
{
    using sync_node_type = typename P::sync_node_type;
    
    using ring_buf_core_type = typename P::ring_buf_core_type;
    using ring_buf_count_type = typename P::ring_buf_count_type;
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    
public:
    basic_ring_buf_queue()
        : nodes_{fdn::make_unique<sync_node_type []>(this->get_capacity())} { }
    
    struct start_lock_result {
        bool            is_locked;
        sync_node_type* prev;
        sync_node_type* cur;
    };
    
    CMPTH_NODISCARD
    start_lock_result start_lock() noexcept
    {
        const auto cap = this->get_capacity();
        while (true) {
            const auto ret = this->core_.try_lock_or_enqueue(cap);
            if (ret.is_locked || ret.is_delegated) {
                const auto prev = & this->get_node_at(ret.prev_tail_idx);
                const auto cur  = & this->get_node_at(ret.cur_tail_idx);
                return { ret.is_locked, prev, cur };
            }
            // Retry.
            base_ult_itf_type::this_thread::yield();
        }
    }
    
    void set_next(const start_lock_result& sl_ret) noexcept {
        const auto prev = sl_ret.prev;
        CMPTH_P_ASSERT(P, !prev->ready.load(fdn::memory_order_relaxed));
        prev->ready.store(true, fdn::memory_order_release);
    }
    
    sync_node_type* get_head() const noexcept {
        const auto cap = this->get_capacity();
        const auto idx = this->core_.get_head(cap);
        return & this->get_node_at(idx);
    }
    
    bool is_unlockable(sync_node_type* const head) noexcept {
        this->check_head(head);
        return this->core_.is_unlockable();
    }
    
    CMPTH_NODISCARD
    bool try_unlock(sync_node_type* const head) noexcept {
        this->check_head(head);
        return this->core_.try_unlock();
    }
    
    CMPTH_NODISCARD
    sync_node_type* try_follow_head(sync_node_type* const head) noexcept {
        this->check_head(head);
        if (head->ready.load(fdn::memory_order_acquire)) {
            // Reset for the next use.
            head->ready.store(false, fdn::memory_order_relaxed);
            
            const auto cap = this->get_capacity();
            const auto next_head_idx = this->core_.follow_head(cap);
            return & this->get_node_at(next_head_idx);
        }
        else {
            return nullptr;
        }
    }
    
private:
    sync_node_type& get_node_at(const ring_buf_count_type idx) const noexcept {
        CMPTH_P_ASSERT(P, 0 <= idx);
        CMPTH_P_ASSERT(P, idx < this->get_capacity());
        return this->nodes_[idx];
    }
    ring_buf_count_type get_index_of(sync_node_type* const head) const noexcept {
        return head - this->nodes_.get();
    }
    void check_head(sync_node_type* const head) const noexcept {
        const auto cap = this->get_capacity();
        const auto head_idx = this->get_index_of(head);
        this->core_.check_head(head_idx, cap);
    }
    ring_buf_count_type get_capacity() const noexcept {
        return P::ring_buf_length; // TODO
    }

    ring_buf_core_type                  core_;
    fdn::unique_ptr<sync_node_type []>  nodes_;
};

} // namespace cmpth

