
#pragma once

#include <mgbase/lang.hpp>
#include "atomic.hpp"

#if (__cplusplus < 201103L)

namespace mgbase {

// In C++03

#if (defined(MGBASE_ARCH_INTEL) || defined(MGBASE_ARCH_SPARC))

// This implementation is available on the architectures with Total Store Ordering (TSO).

class spinlock
    : noncopyable
{
public:
    spinlock() MGBASE_NOEXCEPT : flag_(false) { }
    
    void lock() MGBASE_NOEXCEPT {
        while (true) {
            bool expected = false;
            if (flag_.compare_exchange_weak(expected, true, memory_order_acquire))
                break;
            
            while (!flag_.load(memory_order_relaxed)) {
                // Do nothing.
            }
        }
    }
    
    bool try_lock() MGBASE_NOEXCEPT {
        bool expected = false;
        return flag_.compare_exchange_weak(expected, true, memory_order_acquire);
    }
    
    void unlock() MGBASE_NOEXCEPT {
        flag_.store(false, memory_order_release);
    }
    
private:
    atomic<bool> flag_;
};

#else

#error "Spinlock is not defined!"

#endif

}

#else

// In C++11

#include <atomic>

namespace mgbase {

#define MGBASE_SPINLOCK_TTAS_BACKOFF

#ifdef MGBASE_SPINLOCK_TTAS_BACKOFF

// test-and-test-and-set (TTAS) spinlock with exponential backoff

class spinlock {
public:
    spinlock() noexcept : flag_{false} { }
    spinlock(const spinlock&) = delete;
    
    spinlock& operator = (const spinlock&) = delete;

    void lock() noexcept {
        std::size_t wait = 1;
        while (flag_.exchange(true, std::memory_order_acquire)) {
            do {
                // Wait for a while.
                for (std::size_t i = 0; i < wait; i++) {
                    // Insert a relax instruction. (x86 only)
                    cpu_relax();
                }
                
                //  Exponential backoff
                if (wait < 1024) // TODO: reconfigurable
                    wait *= 2;
            }
            while (flag_);
        }
    }
    
    bool try_lock() noexcept {
        return !flag_.exchange(true, std::memory_order_acquire);
    }
    
    void unlock() noexcept {
        flag_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> flag_;
};

#else

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

#endif

}

#endif

