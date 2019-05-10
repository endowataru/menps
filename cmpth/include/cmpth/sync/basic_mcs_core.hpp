
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_core
{
    using mcs_node_type = typename P::mcs_node_type;
    using atomic_node_ptr_type = typename P::atomic_node_ptr_type;
    
public:
    CMPTH_NODISCARD
    mcs_node_type* start_lock(mcs_node_type* const cur) noexcept
    {
        CMPTH_P_ASSERT(P, cur != nullptr);
        
        cur->next.store(nullptr, fdn::memory_order_relaxed);
        
        const auto prev =
            this->tail_.exchange(cur, fdn::memory_order_acq_rel);
        
        if (prev == nullptr) {
            // This thread locked the mutex immediately.
            
        }
        else {
            // This thread couldn't lock the mutex immediately.
            
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
    
    static mcs_node_type* load_next(
        mcs_node_type* const cur
    ) noexcept
    {
        CMPTH_P_ASSERT(P, cur != nullptr);
        return cur->next.load(fdn::memory_order_acquire);
    }
    
    CMPTH_NODISCARD
    bool try_unlock(mcs_node_type* const head) noexcept
    {
        /*if (this->tail_.load(fdn::memory_order_relaxed) != head) {
            return false;
        }*/
        auto expected = head;
        
        return this->tail_.compare_exchange_strong(
            expected, nullptr, fdn::memory_order_release
        );
    }
    
    bool is_unlockable(mcs_node_type* const head) const noexcept
    {
        return this->tail_.load(fdn::memory_order_relaxed) == head;
    }
    
private:
    atomic_node_ptr_type tail_{nullptr};
};

#if 0
template <typename P>
class basic_mcs_mutex_guard
{
    using mutex_type = typename P::mutex_type;
    using mcs_node_type = typename P::mcs_node_type;
    
public:
    explicit basic_mcs_mutex_guard(mutex_type& mtx)
        : mtx_{mtx}
    {
        this->mtx_.lock(&this->node_);
    }
    ~basic_mcs_mutex_guard() {
        this->mtx_.unlock(&this->node_);
    }
    
    basic_mcs_mutex_guard(const basic_mcs_mutex_guard&) = delete;
    basic_mcs_mutex_guard& operator = (const basic_mcs_mutex_guard&) = delete;
    
private:
    mutex_type&     mtx_;
    mcs_node_type   node_;
};
#endif

} // namespace cmpth

