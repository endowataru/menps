
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

#if (defined(MGBASE_ARCH_INTEL) || defined(MGBASE_ARCH_SPARC))

// Atomic integers for the architectures that guarantee Total Store Order (TSO).

template <typename T>
inline T atomic_load_explicit(const volatile T& obj, memory_order order) MGBASE_NOEXCEPT
{
    switch (order) {
        case memory_order_relaxed:
            // Simply read.
            return obj;
        
        case memory_order_acquire:
        case memory_order_consume: {
            // Issue a load.
            const T result = obj;
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            return result;
        }
        
        case memory_order_seq_cst:
        default: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue a load.
            const T result = obj;
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            return result;
        }
    }
}

template <typename T>
inline void atomic_store_explicit(volatile T& obj, T desired, memory_order order) MGBASE_NOEXCEPT
{
    switch (order) {
        case memory_order_relaxed: {
            // Simply write.
            obj = desired;
            break;
        }
        
        case memory_order_release: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue a store.
            obj = desired;
            break;
        }
        
        case memory_order_seq_cst:
        default: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue a store.
            obj = desired;
            // Prevent the processor from advancing the following reads.
            hard_memory_barrier();
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            break;
        }
    }
}

template <typename T>
inline bool atomic_compare_exchange_weak_explicit(volatile T& obj, T& expected, T desired, memory_order success, memory_order /*failure*/) MGBASE_NOEXCEPT
{
    // TODO: Use the ordering "failure", which is the same or weaker than "success".
    
    const T expected_val = expected;
    T old;
    
    switch (success) {
        case memory_order_relaxed: {
            // Atomically exchange.
            old = __sync_val_compare_and_swap(&obj, expected, desired);
            break;
        }
        
        case memory_order_consume:
        case memory_order_acquire: {
            // Issue an exchange.
            old = __sync_val_compare_and_swap(&obj, expected, desired);
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            break;
        }
        
        case memory_order_release: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue an exchange.
            old = __sync_val_compare_and_swap(&obj, expected, desired);
            break;
        }
        
        case memory_order_seq_cst:
        case memory_order_acq_rel:
        default: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue an exchange.
            old = __sync_val_compare_and_swap(&obj, expected, desired);
            // Prevent the processor from advancing the following reads.
            hard_memory_barrier();
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            break;
        }
    }
    
    if (old == expected_val)
        return true;
    else {
        expected = old;
        return false;
    }
}

template <typename T>
inline bool atomic_compare_exchange_strong_explicit(volatile T& obj, T& expected, T desired, memory_order success, memory_order failure) MGBASE_NOEXCEPT
{
    // Assume that there's no spurious failure.
    return atomic_compare_exchange_weak_explicit(obj, expected, desired, success, failure);
}

template <typename T>
inline T atomic_fetch_add_explicit(volatile T& obj, T arg, memory_order order) MGBASE_NOEXCEPT
{
    switch (order) {
        case memory_order_relaxed: {
            // Atomically add.
            return __sync_fetch_and_add(&obj, arg);
        }
        
        case memory_order_consume:
        case memory_order_acquire: {
            // Issue an addition.
            const T result = __sync_fetch_and_add(&obj, arg);
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            return result;
        }
        
        case memory_order_release: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue an addition.
            return __sync_fetch_and_add(&obj, arg);
        }
        
        case memory_order_seq_cst:
        case memory_order_acq_rel:
        default: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue an addition.
            const T result = __sync_fetch_and_add(&obj, arg);
            // Prevent the processor from advancing the following reads.
            hard_memory_barrier();
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            return result;
        }
    }
}

template <typename T>
inline T atomic_fetch_sub_explicit(volatile T& obj, T arg, memory_order order) MGBASE_NOEXCEPT
{
    switch (order) {
        case memory_order_relaxed: {
            // Atomically subtract.
            return __sync_fetch_and_sub(&obj, arg);
        }
        
        case memory_order_consume:
        case memory_order_acquire: {
            // Issue a subtraction.
            const T result = __sync_fetch_and_sub(&obj, arg);
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            return result;
        }
        
        case memory_order_release: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue a subtraction.
            return __sync_fetch_and_sub(&obj, arg);
        }
        
        case memory_order_seq_cst:
        case memory_order_acq_rel:
        default: {
            // Prevent the compiler from postponing the preceding operations.
            soft_memory_barrier();
            // Issue a subtraction.
            const T result = __sync_fetch_and_sub(&obj, arg);
            // Prevent the processor from advancing the following reads.
            hard_memory_barrier();
            // Prevent the compiler from advancing the following operations.
            soft_memory_barrier();
            return result;
        }
    }
}

#else

#error "Atomic types are not defined in this platform."

#endif


template <typename T>
inline T atomic_load(const volatile T& obj) MGBASE_NOEXCEPT {
    return atomic_load_explicit(obj, memory_order_seq_cst);
}
template <typename T>
inline void atomic_store(volatile T& obj, T desired) MGBASE_NOEXCEPT {
    atomic_store_explicit(obj, desired, memory_order_seq_cst);
}
template <typename T>
inline bool atomic_compare_exchange_weak(volatile T& obj, T& expected, T desired) MGBASE_NOEXCEPT {
    return atomic_compare_exchange_weak_explicit(obj, expected, desired, memory_order_seq_cst, memory_order_seq_cst);
}
template <typename T>
inline bool atomic_compare_exchange_strong(volatile T& obj, T& expected, T desired) MGBASE_NOEXCEPT {
    return atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order_seq_cst, memory_order_seq_cst);
}
template <typename T>
inline T atomic_fetch_add(volatile T& obj, T arg) MGBASE_NOEXCEPT {
    return atomic_fetch_add_explicit(obj, arg, memory_order_seq_cst);
}
template <typename T>
inline T atomic_fetch_sub(volatile T& obj, T arg) MGBASE_NOEXCEPT {
    return atomic_fetch_sub_explicit(obj, arg, memory_order_seq_cst);
}

template <typename T>
class atomic
    : noncopyable
{
public:
    atomic() MGBASE_NOEXCEPT : value_() { }
    explicit atomic(T val) MGBASE_NOEXCEPT
        : value_(val) { }
    
    volatile atomic& operator = (T val) volatile MGBASE_NOEXCEPT {
        store(val);
        return *this;
    }
    
    T load(memory_order order = memory_order_seq_cst) const volatile MGBASE_NOEXCEPT {
        return atomic_load_explicit(value_, order);
    }
    
    void store(T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        atomic_store_explicit(value_, desired, order);
    }
    
    T compare_exchange_weak(T& expected, T desired, memory_order success, memory_order failure) volatile MGBASE_NOEXCEPT {
        return atomic_compare_exchange_weak_explicit(value_, expected, desired, success, failure);
    }
    T compare_exchange_weak(T& expected, T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return compare_exchange_weak(expected, desired, order, order);
    }
    
    T compare_exchange_strong(T& expected, T desired, memory_order success, memory_order failure) volatile MGBASE_NOEXCEPT {
        return atomic_compare_exchange_strong_explicit(value_, expected, desired, success, failure);
    }
    T compare_exchange_strong(T& expected, T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return compare_exchange_strong(expected, desired, order, order);
    }
    
    T fetch_add(T arg, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return atomic_fetch_add_explicit(value_, arg, order);
    }
    T fetch_sub(T arg, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return atomic_fetch_sub_explicit(value_, arg, order);
    }
    
private:
    volatile T value_;
};


}

