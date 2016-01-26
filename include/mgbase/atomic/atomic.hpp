
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/memory_order.hpp>
#include <mgbase/atomic/memory_barrier.hpp>

namespace mgbase {

//#define MGBASE_ATOMIC_FORCE_SEQ_CST

// Reference: Boost.Atomic

namespace detail
{
    // Note: Assume Total Store Order (TSO).
    
    MGBASE_ALWAYS_INLINE void fence_before_load(const memory_order /*order*/) MGBASE_NOEXCEPT
    {
        // do nothing
    }
    
    MGBASE_ALWAYS_INLINE void fence_after_load(const memory_order order) MGBASE_NOEXCEPT
    {
        // If the memory order is acquire or consume or seq_cst,
        // prevent from advancing the following operations.
        
        #ifdef MGBASE_ARCH_SPARC
            // SPARC
            if (order == memory_order_seq_cst)
                __asm__ __volatile__ ("membar #Sync" ::: "memory");
            else if ((order & (memory_order_consume | memory_order_acquire)) != 0)
                __asm__ __volatile__ ("membar #StoreStore | #LoadStore" ::: "memory");
        #else
            // Portable implementation
            if ((order & (memory_order_acquire | memory_order_consume)) != 0)
                __sync_synchronize();
        #endif
    }
    
    MGBASE_ALWAYS_INLINE void fence_before_store(const memory_order order) MGBASE_NOEXCEPT
    {
        // If the memory order is release or seq_cst,
        // prevent from postponing the preceding operations.
        
        #ifdef MGBASE_ARCH_SPARC
            // SPARC
            if (order == memory_order_seq_cst)
                __asm__ __volatile__ ("membar #Sync" ::: "memory");
            else if ((order & memory_order_release) != 0)
                __asm__ __volatile__ ("membar #StoreStore | #LoadStore" ::: "memory");
        #else
            // Portable implementation
            if ((order & memory_order_release) != 0)
                __sync_synchronize();
        #endif
    }
    
    MGBASE_ALWAYS_INLINE void fence_after_store(const memory_order order) MGBASE_NOEXCEPT
    {
        // If the memory order is acquire or consume or seq_cst,
        // prevent from advancing the following operations.
        
        #ifdef MGBASE_ARCH_SPARC
            // SPARC
            if (order == memory_order_seq_cst)
                __asm__ __volatile__ ("membar #Sync" ::: "memory");
            
        #else
            // Portable implementation
            if (order == memory_order_seq_cst)
                __sync_synchronize();
        #endif
    }
}

template <typename T>
MGBASE_ALWAYS_INLINE T atomic_load_explicit(const volatile T* const obj, /*const*/ memory_order order) MGBASE_NOEXCEPT
{
    #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
        order = memory_order_seq_cst;
    #endif
    
    // Fence before load.
    detail::fence_before_load(order);
    
    // Issue load.
    const T ret = *obj;
    
    // Fence after load.
    detail::fence_after_load(order);
    
    return ret;
}

template <typename T>
MGBASE_ALWAYS_INLINE void atomic_store_explicit(volatile T* const obj, const T desired, /*const*/ memory_order order) MGBASE_NOEXCEPT
{
    #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
        order = memory_order_seq_cst;
    #endif
    
    // Fence before store.
    detail::fence_before_store(order);
    
    // Issue store.
    *obj = desired;
    
    // Fence before store.
    detail::fence_after_store(order);
}

template <typename T>
MGBASE_ALWAYS_INLINE bool atomic_compare_exchange_weak_explicit(
    volatile T* const   obj
,   T* const            expected
,   const T             desired
,   const memory_order  //success
,   memory_order        //failure
) MGBASE_NOEXCEPT
{
    const T expected_val = *expected;
    const T old =  __sync_val_compare_and_swap(obj, expected_val, desired);
    
    if (old == expected_val)
        return true;
    else {
        *expected = old;
        return false;
    }
}

template <typename T>
MGBASE_ALWAYS_INLINE bool atomic_compare_exchange_strong_explicit(
    volatile T* const   obj
,   T* const            expected
,   const T             desired
,   memory_order        success
,   memory_order        failure
) MGBASE_NOEXCEPT
{
    // Assume that there's no spurious failure.
    return atomic_compare_exchange_weak_explicit(obj, expected, desired, success, failure);
}

template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_add_explicit(volatile T* const obj, const T arg, const memory_order /*order*/) MGBASE_NOEXCEPT
{
    const T old = __sync_fetch_and_add(obj, arg);
    return old;
}

template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_sub_explicit(volatile T* const obj, const T arg, const memory_order /*order*/) MGBASE_NOEXCEPT
{
    const T old = __sync_fetch_and_sub(obj, arg);
    return old;
}

template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_and_explicit(volatile T* const obj, const T arg, const memory_order /*order*/) MGBASE_NOEXCEPT
{
    const T old = __sync_fetch_and_and(obj, arg);
    return old;
}

template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_or_explicit(volatile T* const obj, const T arg, const memory_order /*order*/) MGBASE_NOEXCEPT
{
    const T old = __sync_fetch_and_or(obj, arg);
    return old;
}

template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_xor_explicit(volatile T* const obj, const T arg, const memory_order /*order*/) MGBASE_NOEXCEPT
{
    const T old = __sync_fetch_and_xor(obj, arg);
    return old;
}
template <typename T>
MGBASE_ALWAYS_INLINE T atomic_load(const volatile T* obj) MGBASE_NOEXCEPT {
    return atomic_load_explicit(obj, memory_order_seq_cst);
}
template <typename T>
MGBASE_ALWAYS_INLINE void atomic_store(volatile T* obj, T desired) MGBASE_NOEXCEPT {
    atomic_store_explicit(obj, desired, memory_order_seq_cst);
}
template <typename T>
MGBASE_ALWAYS_INLINE bool atomic_compare_exchange_weak(volatile T* obj, T* expected, T desired) MGBASE_NOEXCEPT {
    return atomic_compare_exchange_weak_explicit(obj, expected, desired, memory_order_seq_cst, memory_order_seq_cst);
}
template <typename T>
MGBASE_ALWAYS_INLINE bool atomic_compare_exchange_strong(volatile T* obj, T* expected, T desired) MGBASE_NOEXCEPT {
    return atomic_compare_exchange_strong_explicit(obj, expected, desired, memory_order_seq_cst, memory_order_seq_cst);
}
template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_add(volatile T* obj, T arg) MGBASE_NOEXCEPT {
    return atomic_fetch_add_explicit(obj, arg, memory_order_seq_cst);
}
template <typename T>
MGBASE_ALWAYS_INLINE T atomic_fetch_sub(volatile T* obj, T arg) MGBASE_NOEXCEPT {
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
    
    MGBASE_ALWAYS_INLINE T load(memory_order order = memory_order_seq_cst) const volatile MGBASE_NOEXCEPT {
        return atomic_load_explicit(&value_, order);
    }
    
    MGBASE_ALWAYS_INLINE void store(T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        atomic_store_explicit(&value_, desired, order);
    }
    
    MGBASE_ALWAYS_INLINE T compare_exchange_weak(T& expected, T desired, memory_order success, memory_order failure) volatile MGBASE_NOEXCEPT {
        return atomic_compare_exchange_weak_explicit(&value_, &expected, desired, success, failure);
    }
    MGBASE_ALWAYS_INLINE T compare_exchange_weak(T& expected, T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return compare_exchange_weak(expected, desired, order, order);
    }
    
    MGBASE_ALWAYS_INLINE T compare_exchange_strong(T& expected, T desired, memory_order success, memory_order failure) volatile MGBASE_NOEXCEPT {
        return atomic_compare_exchange_strong_explicit(&value_, &expected, desired, success, failure);
    }
    MGBASE_ALWAYS_INLINE T compare_exchange_strong(T& expected, T desired, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return compare_exchange_strong(expected, desired, order, order);
    }
    
    MGBASE_ALWAYS_INLINE T fetch_add(T arg, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return atomic_fetch_add_explicit(&value_, arg, order);
    }
    MGBASE_ALWAYS_INLINE T fetch_sub(T arg, memory_order order = memory_order_seq_cst) volatile MGBASE_NOEXCEPT {
        return atomic_fetch_sub_explicit(&value_, arg, order);
    }
    
private:
    volatile T value_;
};

} // namespace mgbase

