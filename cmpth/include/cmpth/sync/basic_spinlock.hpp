
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

// Implementation using compare_exchange_weak

template <typename P>
class basic_spinlock
{
    using atomic_bool_type = typename P::atomic_bool_type;
    
public:
    basic_spinlock() noexcept = default;
    
    basic_spinlock(const basic_spinlock&) = delete;
    basic_spinlock& operator = (const basic_spinlock&) = delete;
    
    void lock() noexcept
    {
        while (true)
        {
            bool expected = false;
            
            const auto unlocked =
                this->flag_.compare_exchange_weak(
                    expected, true, fdn::memory_order_acquire);
            
            if (CMPTH_LIKELY(unlocked)) { break; }
            
            // Wait until flag_ becomes false.
            while (this->flag_.load(fdn::memory_order_relaxed)) {
                // Do nothing.
            }
        }
    }
    
    bool try_lock() noexcept
    {
        bool expected = false;
        
        return this->flag_.compare_exchange_weak(
            expected, true, fdn::memory_order_acquire);
    }
    
    void unlock() noexcept
    {
        CMPTH_P_ASSERT(P, this->flag_.load(fdn::memory_order_relaxed));
        this->flag_.store(false, fdn::memory_order_release);
    }
    
private:
    atomic_bool_type flag_{false};
};

} // namespace cmpth

