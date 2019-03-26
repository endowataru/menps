
#pragma once

#include <mgbase/memory_order.hpp>
#include <mgbase/type_traits.hpp>
#include <mgbase/atomic/atomic_base.h>

namespace mgbase {

// Reference: Boost.Atomic

namespace detail {

// Use the original type.

template <typename T>
struct atomic_storage
{
    typedef T   type;
};

// Note: Assume Total Store Order (TSO).

MGBASE_ALWAYS_INLINE
void fence_before(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is release or seq_cst,
    // prevent from postponing the preceding operations.
    
    // TSO only
    if ((order & memory_order_release) != 0)
        __asm__ __volatile__ ("" ::: "memory");
}

MGBASE_ALWAYS_INLINE
void fence_after(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is acquire or consume or seq_cst,
    // prevent from advancing the following operations.
    
    // TSO only
    if ((order & memory_order_acquire) != 0)
        __asm__ __volatile__ ("" ::: "memory");
}

// Load

template <typename T>
MGBASE_ALWAYS_INLINE
T load(const volatile T* const obj, const memory_order order) MGBASE_NOEXCEPT
{
    // We don't need fence before load.
    
    // Issue load.
    const T ret = *obj;
    
    // Fence after load.
    fence_after(order);
    
    return ret;
}

// Store

template <typename T>
MGBASE_ALWAYS_INLINE
void store(volatile T* const obj, const T desired, const memory_order order) MGBASE_NOEXCEPT
{
    // Fence before store.
    fence_before(order);
    
    // Issue store.
    *obj = desired;
    
    // Fence before store.
    fence_after(order);
}

// Compare-and-swap

MGBASE_ALWAYS_INLINE
bool compare_exchange_weak(
    volatile mgbase::uint8_t* const     obj
,   mgbase::uint8_t* const              expected
,   mgbase::uint8_t                     desired // modified
,   const memory_order                  //success
,   const memory_order                  //failure
) MGBASE_NOEXCEPT
{
    mgbase::uint8_t previous = *expected;
    bool result;
    __asm__ __volatile__ (
        "lock; cmpxchgb %3, %1\n\t"
        "sete %2"
        : "+a" (previous), "+m" (*obj), "=q" (result)
        : "q" (desired)
        : "cc", "memory"
    );
    *expected = previous;
    return result;
}

MGBASE_ALWAYS_INLINE
bool compare_exchange_weak(
    volatile mgbase::uint16_t* const    obj
,   mgbase::uint16_t* const             expected
,   mgbase::uint16_t                    desired // modified
,   const memory_order                  //success
,   const memory_order                  //failure
) MGBASE_NOEXCEPT
{
    mgbase::uint16_t previous = *expected;
    bool result;
    __asm__ __volatile__ (
        "lock; cmpxchgw %3, %1\n\t"
        "sete %2"
        : "+a" (previous), "+m" (*obj), "=q" (result)
        : "q" (desired)
        : "cc", "memory"
    );
    *expected = previous;
    return result;
}

MGBASE_ALWAYS_INLINE
bool compare_exchange_weak(
    volatile mgbase::uint32_t* const    obj
,   mgbase::uint32_t* const             expected
,   mgbase::uint32_t                    desired // modified
,   const memory_order                  //success
,   const memory_order                  //failure
) MGBASE_NOEXCEPT
{
    mgbase::uint32_t previous = *expected;
    bool result;
    __asm__ __volatile__ (
        "lock; cmpxchgl %3, %1\n\t"
        "sete %2"
        : "+a" (previous), "+m" (*obj), "=q" (result)
        : "q" (desired)
        : "cc", "memory"
    );
    *expected = previous;
    return result;
}

MGBASE_ALWAYS_INLINE
bool compare_exchange_weak(
    volatile mgbase::uint64_t* const    obj
,   mgbase::uint64_t* const             expected
,   mgbase::uint64_t                    desired // modified
,   const memory_order                  //success
,   const memory_order                  //failure
) MGBASE_NOEXCEPT
{
    mgbase::uint64_t previous = *expected;
    bool result;
    __asm__ __volatile__ (
        "lock; cmpxchgq %3, %1\n\t"
        "sete %2"
        : "+a" (previous), "+m" (*obj), "=q" (result)
        : "q" (desired)
        : "cc", "memory"
    );
    *expected = previous;
    return result;
}

template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE
bool compare_exchange_strong(
    volatile StorageT* const    obj
,   T* const                    expected
,   const T                     desired
,   const memory_order          success
,   const memory_order          failure
) MGBASE_NOEXCEPT
{
    // Assume that there's no spurious failure.
    return compare_exchange_weak(obj, expected, desired, success, failure);
}

// Fetch-and-add

MGBASE_ALWAYS_INLINE
mgbase::uint8_t fetch_add(
    volatile mgbase::uint8_t* const     obj
,   mgbase::uint8_t                     arg
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__(
        "lock; xaddb %0, %1"
        : "+q" (arg), "+m" (*obj)
        :
        : "cc", "memory"
    );
    return arg;
}

MGBASE_ALWAYS_INLINE
mgbase::uint16_t fetch_add(
    volatile mgbase::uint16_t* const    obj
,   mgbase::uint16_t                    arg
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__(
        "lock; xaddw %0, %1"
        : "+q" (arg), "+m" (*obj)
        :
        : "cc", "memory"
    );
    return arg;
}

MGBASE_ALWAYS_INLINE
mgbase::uint32_t fetch_add(
    volatile mgbase::uint32_t* const    obj
,   mgbase::uint32_t                    arg
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__(
        "lock; xaddl %0, %1"
        : "+q" (arg), "+m" (*obj)
        :
        : "cc", "memory"
    );
    return arg;
}

MGBASE_ALWAYS_INLINE
mgbase::uint64_t fetch_add(
    volatile mgbase::uint64_t* const    obj
,   mgbase::uint64_t                    arg
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__(
        "lock; xaddq %0, %1"
        : "+q" (arg), "+m" (*obj)
        :
        : "cc", "memory"
    );
    return arg;
}

// Fetch-and-sub

template <typename T>
MGBASE_ALWAYS_INLINE
T fetch_sub(
    volatile T* const   obj
,   const T             arg
,   const memory_order  order
) MGBASE_NOEXCEPT
{
    return fetch_add(obj, -arg, order);
}

// Fetch and op
// Fallback to CAS

#define DEFINE_FETCH_OP(name, NAME, c_op) \
    template <typename T> \
    MGBASE_ALWAYS_INLINE \
    T fetch_##name(volatile T* const obj, const T arg, const memory_order order) MGBASE_NOEXCEPT \
    { \
        while (true) { \
            const T old = *obj; \
            T expected = old; \
            const T desired = expected c_op arg; \
            if (MGBASE_LIKELY( \
                compare_exchange_weak(obj, &expected, desired, order, order) \
            )) \
            { return old; } \
        } \
    }


DEFINE_FETCH_OP(and, AND, &)
DEFINE_FETCH_OP(xor, XOR, ^)
DEFINE_FETCH_OP(or , OR , |)

#undef DEFINE_FETCH_OP

// Exchange

MGBASE_ALWAYS_INLINE
mgbase::uint8_t exchange(
    volatile mgbase::uint8_t* const     obj
,   mgbase::uint8_t                     desired
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__
    (
        "xchgb %0, %1"
        : "+q" (desired), "+m" (*obj)
        :
        : "memory"
    );
    return desired;
}

MGBASE_ALWAYS_INLINE
mgbase::uint16_t exchange(
    volatile mgbase::uint16_t* const    obj
,   mgbase::uint16_t                    desired
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__
    (
        "xchgw %0, %1"
        : "+q" (desired), "+m" (*obj)
        :
        : "memory"
    );
    return desired;
}

MGBASE_ALWAYS_INLINE
mgbase::uint32_t exchange(
    volatile mgbase::uint32_t* const    obj
,   mgbase::uint32_t                    desired
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__
    (
        "xchgl %0, %1"
        : "+q" (desired), "+m" (*obj)
        :
        : "memory"
    );
    return desired;
}

MGBASE_ALWAYS_INLINE
mgbase::uint64_t exchange(
    volatile mgbase::uint64_t* const    obj
,   mgbase::uint64_t                    desired
,   const memory_order                  //order
) MGBASE_NOEXCEPT
{
    __asm__ __volatile__
    (
        "xchgq %0, %1"
        : "+q" (desired), "+m" (*obj)
        :
        : "memory"
    );
    return desired;
}

} // namespace detail

} // namespace mgbase

