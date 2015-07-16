
#pragma once

#include <mgbase/lang.hpp>
#include "memory_barrier.hpp"

namespace mgbase {

enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
};

namespace detail {

#if (defined(MGBASE_ARCH_INTEL) || defined(MGBASE_ARCH_SPARC))

// Atomic integers for the architectures that guarantee Total Store Order (TSO).

template <typename T>
class atomic_integer
    : noncopyable
{
protected:
    explicit atomic_integer(T val)
        : value_(val) { }

public:
    /*implicit*/ operator T () const MGBASE_NOEXCEPT {
        return load(memory_order_seq_cst);
    }
    
    T load(memory_order order = memory_order_seq_cst) const volatile MGBASE_NOEXCEPT {
        switch (order) {
            case memory_order_relaxed:
                // Simply read.
                return value_;
            
            case memory_order_acquire:
            case memory_order_consume: {
                // Issue a load.
                const T result = value_;
                // Prevent the compiler from advancing the following operations.
                soft_memory_barrier();
                return result;
            }
            
            case memory_order_seq_cst:
            default: {
                // Prevent the compiler from postponing the preceding operations.
                soft_memory_barrier();
                // Issue a load.
                const T result = value_;
                // Prevent the compiler from advancing the following operations.
                soft_memory_barrier();
                return result;
            }
        }
    }
    
    void store(T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        switch (order) {
            case memory_order_relaxed: {
                // Simply write.
                value_ = desired;
                break;
            }
            
            case memory_order_release: {
                // Prevent the compiler from postponing the preceding operations.
                soft_memory_barrier();
                // Issue a store.
                value_ = desired;
                break;
            }
            
            case memory_order_seq_cst:
            default: {
                // Prevent the compiler from postponing the preceding operations.
                soft_memory_barrier();
                // Issue a store.
                value_ = desired;
                // Prevent the processor from advancing the following reads.
                hard_memory_barrier();
                // Prevent the compiler from advancing the following operations.
                soft_memory_barrier();
                break;
            }
        }
    }
    
    T compare_exchange_weak(T& expected, T desired, memory_order success, memory_order /*failure*/) volatile MGBASE_NOEXCEPT {
        // TODO: Use the ordering "failure", which is the same or weaker than "success".
        switch (success) {
            case memory_order_relaxed: {
                // Atomically exchange.
                return __sync_val_compare_and_swap(&value_, expected, desired);
            }
            
            case memory_order_consume:
            case memory_order_acquire: {
                // Issue an exchange.
                const T result = __sync_val_compare_and_swap(&value_, expected, desired);
                // Prevent the compiler from advancing the following operations.
                soft_memory_barrier();
                
                return result;
            }
            
            case memory_order_release: {
                // Prevent the compiler from postponing the preceding operations.
                soft_memory_barrier();
                // Issue an exchange.
                return __sync_val_compare_and_swap(&value_, expected, desired);
            }
            
            case memory_order_seq_cst:
            case memory_order_acq_rel:
            default: {
                // Prevent the compiler from postponing the preceding operations.
                soft_memory_barrier();
                // Issue an exchange.
                const T result = __sync_val_compare_and_swap(&value_, expected, desired);
                // Prevent the processor from advancing the following reads.
                hard_memory_barrier();
                // Prevent the compiler from advancing the following operations.
                soft_memory_barrier();
                break;
            }
        }
    }
    T compare_exchange_weak(T& expected, T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return compare_exchange_weak(expected, desired, order, order);
    }
    
    T compare_exchange_strong(T& expected, T desired, memory_order success, memory_order failure) volatile MGBASE_NOEXCEPT {
        /*
         * Assume that there's no spurious failure.
         */
        return compare_exchange_weak(expected, desired, success, failure);
    }
    T compare_exchange_strong(T& expected, T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return compare_exchange_strong(expected, desired, order, order);
    }
    
private:
    volatile T value_;
};

#else

#error "Atomic types are not defined in this platform."

#endif

}

template <typename T>
class atomic;

template <>
class atomic<bool>
    : public detail::atomic_integer<bool>
{
    typedef detail::atomic_integer<bool>  base;
    
public:
    atomic() : base(false) { }
    atomic(bool val) : base(val) { }
};

}

