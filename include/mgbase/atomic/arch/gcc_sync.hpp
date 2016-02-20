
#pragma once

#include <mgbase/memory_order.hpp>
#include <mgbase/atomic/atomic_base.h>

namespace mgbase {

//#define MGBASE_ATOMIC_FORCE_SEQ_CST

// Reference: Boost.Atomic

namespace detail {

// Use the original type.

template <typename T>
struct atomic_storage
{
    typedef T   type;
};

// Note: Assume Total Store Order (TSO).

MGBASE_ALWAYS_INLINE void fence_before_load(const memory_order /*order*/) MGBASE_NOEXCEPT
{
    // do nothing
}

MGBASE_ALWAYS_INLINE void fence_after_load(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is acquire or consume or seq_cst,
    // prevent from advancing the following operations.
    
    // Portable implementation
    if ((order & (memory_order_acquire | memory_order_consume)) != 0)
        __sync_synchronize();
}

MGBASE_ALWAYS_INLINE void fence_before_store(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is release or seq_cst,
    // prevent from postponing the preceding operations.
    
    // Portable implementation
    if ((order & memory_order_release) != 0)
        __sync_synchronize();
}

MGBASE_ALWAYS_INLINE void fence_after_store(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is acquire or consume or seq_cst,
    // prevent from advancing the following operations.
    
    // Portable implementation
    if (order == memory_order_seq_cst)
        __sync_synchronize();
}

template <typename T>
MGBASE_ALWAYS_INLINE T load(const volatile T* const obj, /*const*/ memory_order order) MGBASE_NOEXCEPT
{
    #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
        order = memory_order_seq_cst;
    #endif
    
    // Fence before load.
    fence_before_load(order);
    
    // Issue load.
    const T ret = *obj;
    
    // Fence after load.
    fence_after_load(order);
    
    return ret;
}

template <typename T>
MGBASE_ALWAYS_INLINE void store(volatile T* const obj, const T desired, /*const*/ memory_order order) MGBASE_NOEXCEPT
{
    #ifdef MGBASE_ATOMIC_FORCE_SEQ_CST
        order = memory_order_seq_cst;
    #endif
    
    // Fence before store.
    fence_before_store(order);
    
    // Issue store.
    *obj = desired;
    
    // Fence before store.
    fence_after_store(order);
}

template <typename T>
MGBASE_ALWAYS_INLINE bool compare_exchange_weak(
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
MGBASE_ALWAYS_INLINE bool compare_exchange_strong(
    volatile T* const   obj
,   T* const            expected
,   const T             desired
,   memory_order        success
,   memory_order        failure
) MGBASE_NOEXCEPT
{
    // Assume that there's no spurious failure.
    return compare_exchange_weak(obj, expected, desired, success, failure);
}

#define DEFINE_FETCH_OP(op, OP) \
    template <typename T> \
    MGBASE_ALWAYS_INLINE T fetch_##op(volatile T* const obj, const T arg, const memory_order /*order*/) MGBASE_NOEXCEPT \
    { \
        const T old = __sync_fetch_and_##op(obj, arg); \
        return old; \
    }

MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)

#undef DEFINE_FETCH_OP

} // namespace detail

} // namespace mgbase

