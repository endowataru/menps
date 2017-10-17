
#pragma once

#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace mefdn {

// Implementation using compare_exchange_weak

class spinlock
{
public:
    spinlock() noexcept {
        flag_.store(false, mefdn::memory_order_relaxed);
    }
    
    spinlock(const spinlock&) = delete;
    spinlock& operator = (const spinlock&) = delete;
    
    void lock() noexcept {
        while (true) {
            bool expected = false;
            if (flag_.compare_exchange_weak(expected, true, memory_order_acquire))
                break;
            
            // Wait until flag_ becomes false.
            while (flag_.load(memory_order_relaxed)) {
                // Do nothing.
            }
        }
    }
    
    bool try_lock() noexcept {
        bool expected = false;
        return flag_.compare_exchange_weak(expected, true, memory_order_acquire);
    }
    
    void unlock() noexcept {
        MEFDN_ASSERT(flag_.load(mefdn::memory_order_relaxed));
        flag_.store(false, memory_order_release);
    }
    
private:
    atomic<bool> flag_;
};

#if 0

// test-and-test-and-set (TTAS) spinlock with exponential backoff

class spinlock
    : noncopyable
{
public:
    spinlock() noexcept
    #ifdef MEFDN_CXX11_SUPPORTED
        : flag_(false) { }
    #else
    {
        flag_.store(false, mefdn::memory_order_relaxed);
    }
    #endif
    
    void lock() noexcept
    {
        mefdn::size_t wait = 1;
        
        while (flag_.exchange(true, mefdn::memory_order_acquire))
        {
            do {
                // Wait for a while.
                for (mefdn::size_t i = 0; i < wait; i++)
                {
                    // Insert a relax instruction. (x86 only)
                    //cpu_relax(); // TODO
                }
                
                //  Exponential backoff
                if (wait < 1024) // TODO: reconfigurable
                    wait *= 2;
            }
            while (flag_.load(mefdn::memory_order_relaxed));
        }
    }
    
    bool try_lock() noexcept
    {
        return !flag_.exchange(true, mefdn::memory_order_acquire);
    }
    
    void unlock() noexcept
    {
        flag_.store(false, mefdn::memory_order_release);
    }

private:
    mefdn::atomic<bool> flag_;
};

#endif

#if 0

// The most simple spinlock (but not scalable)

class spinlock {
public:
    spinlock() noexcept : flag_{ATOMIC_FLAG_INIT} { }
    spinlock(const spinlock&) = delete;
    
    spinlock& operator = (const spinlock&) = delete;

    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire)) { }
    }
    
    bool try_lock() noexcept {
        return !flag_.test_and_set(std::memory_order_acquire);
    }
    
    void unlock() noexcept {
        flag_.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag_;
};

#endif // MEFDN_SPINLOCK_TTAS_BACKOFF

} // namespace mefdn
} // namespace menps
 
