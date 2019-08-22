
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_queue
{
    using sync_node_type = typename P::sync_node_type;

    using mcs_core_type = typename P::mcs_core_type;
    using mcs_node_pool_type = typename P::mcs_node_pool_type;

public:
    struct start_lock_result {
        bool            is_locked;
        sync_node_type* prev;
        sync_node_type* cur;
    };
    
    CMPTH_NODISCARD
    start_lock_result start_lock() noexcept {
        const auto cur = this->pool_.allocate();
        const auto prev = this->core_.start_lock(cur);
        const auto is_locked = prev == nullptr;
        return { is_locked, prev, cur };
    }
    
    void set_next(const start_lock_result& sl_ret) noexcept {
        this->core_.set_next(sl_ret.prev, sl_ret.cur);
    }
    
    sync_node_type* get_head() const noexcept {
        return this->core_.get_head();
    }
    
    bool is_unlockable(sync_node_type* const head) noexcept {
        return this->core_.is_unlockable(head);
    }
    
    CMPTH_NODISCARD
    bool try_unlock(sync_node_type* const head) noexcept {
        const auto ret = this->core_.try_unlock(head);
        if (ret) {
            this->pool_.deallocate(head);
        }
        return ret;
    }
    
    CMPTH_NODISCARD
    sync_node_type* try_follow_head(sync_node_type* const head) noexcept {
        const auto ret = this->core_.try_follow_head(head);
        if (ret) {
            this->pool_.deallocate(head);
        }
        return ret;
    }
    
private:
    mcs_core_type       core_;
    mcs_node_pool_type  pool_;
};

} // namespace cmpth

