
#pragma once

#include <mgbase/memory_order.hpp>
#include <mgbase/type_traits.hpp>
#include <mgbase/atomic/atomic_base.h>

namespace mgbase {

// Reference: Boost.Atomic

namespace detail {

template <typename StorageT, typename T>
MGBASE_ALWAYS_INLINE StorageT cast_to_atomic_storage(const /*integer*/ T value) {
    // Change signedness and implicit cast.
    return static_cast<typename mgbase::make_unsigned<T>::type>(value);
}
template <typename StorageT>
MGBASE_ALWAYS_INLINE StorageT cast_to_atomic_storage(const bool value) {
    // Implicitly cast.
    return value;
}
template <typename StorageT, typename T>
MGBASE_ALWAYS_INLINE StorageT cast_to_atomic_storage(T* const ptr) {
    // Cast a pointer to an integer.
    return reinterpret_cast<StorageT>(ptr);
}

template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE typename mgbase::enable_if<
    !mgbase::is_pointer<T>::value
,   T
>::type
cast_from_atomic_storage(const StorageT storage) {
    // Shrink to the original type.
    return static_cast<T>(storage);
}
template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE typename mgbase::enable_if<
    mgbase::is_pointer<T>::value
,   T
>::type
cast_from_atomic_storage(const StorageT storage) {
    // Cast an integer to a pointer.
    return reinterpret_cast<T>(storage);
}


template <typename T>
struct atomic_storage
{
    MGBASE_STATIC_ASSERT((sizeof(T) <= sizeof(mgbase::uint64_t)));
    
    typedef typename mgbase::conditional<
        (sizeof(T) <= sizeof(mgbase::uint32_t))
    ,   mgbase::uint32_t
    ,   mgbase::uint64_t
    >::type
    type;
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
    
    // SPARC64 IXfx (TSO only)
    if (order == memory_order_seq_cst)
        __asm__ __volatile__ ("membar #Sync" ::: "memory");
}

MGBASE_ALWAYS_INLINE void fence_before_store(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is release or seq_cst,
    // prevent from postponing the preceding operations.
    
    // SPARC64 IXfx (TSO only)
    if (order == memory_order_seq_cst)
        __asm__ __volatile__ ("membar #Sync" ::: "memory");
}

MGBASE_ALWAYS_INLINE void fence_after_store(const memory_order order) MGBASE_NOEXCEPT
{
    // If the memory order is acquire or consume or seq_cst,
    // prevent from advancing the following operations.
    
    // SPARC64 IXfx (TSO only)
    if (order == memory_order_seq_cst)
        __asm__ __volatile__ ("membar #Sync" ::: "memory");
}

// Load

template <typename T>
MGBASE_ALWAYS_INLINE T load_impl(const volatile T* const obj, /*const*/ memory_order order) MGBASE_NOEXCEPT
{
    // Fence before load.
    fence_before_load(order);
    
    // Issue load.
    const T ret = *obj;
    
    // Fence after load.
    fence_after_load(order);
    
    return ret;
}

template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE T load(const volatile StorageT* const obj, const memory_order order) MGBASE_NOEXCEPT
{
    // Shrink to the original type.
    return cast_from_atomic_storage<T>(load_impl(obj, order));
}

// Store

template <typename T>
MGBASE_ALWAYS_INLINE void store_impl(volatile T* const obj, const T desired, const memory_order order) MGBASE_NOEXCEPT
{
    // Fence before store.
    fence_before_store(order);
    
    // Issue store.
    *obj = desired;
    
    // Fence before store.
    fence_after_store(order);
}

template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE void store(volatile StorageT* const obj, const T desired, const memory_order order) MGBASE_NOEXCEPT
{
    // Expand to the storage type.
    store_impl(obj, cast_to_atomic_storage<StorageT>(desired), order);
}

// Compare and swap

MGBASE_ALWAYS_INLINE bool compare_exchange_weak_impl(
    volatile mgbase::uint32_t* const    obj
,   mgbase::uint32_t* const             expected
,   mgbase::uint32_t                    desired // modified
,   const memory_order                  success
,   const memory_order                  failure
) MGBASE_NOEXCEPT
{
    // Fence before store.
    fence_before_store(success);
    
    // Load the expected value.
    const mgbase::uint32_t expected_val = *expected;
    
    __asm__ __volatile__ (
        "cas [%1], %2, %0"
        : "+r"(desired)
        : "r"(obj), "r"(expected_val)
        : "memory"
    );
    
    const bool result = desired == expected_val;
    
    // Fence after store.
    fence_after_store(result ? success : failure);
    
    *expected = desired;
    
    return result;
}

MGBASE_ALWAYS_INLINE bool compare_exchange_weak_impl(
    volatile mgbase::uint64_t* const    obj
,   mgbase::uint64_t* const             expected
,   mgbase::uint64_t                    desired // modified
,   const memory_order                  success
,   const memory_order                  failure
) MGBASE_NOEXCEPT
{
    // Fence before store.
    fence_before_store(success);
    
    // Load the expected value.
    const mgbase::uint64_t expected_val = *expected;
    
    __asm__ __volatile__ (
        "casx [%1], %2, %0"
        : "+r"(desired)
        : "r"(obj), "r"(expected_val)
        : "memory"
    );
    
    const bool result = desired == expected_val;
    
    // Fence after store.
    fence_after_store(result ? success : failure);
    
    *expected = desired;
    
    return result;
}

template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE bool compare_exchange_weak(
    volatile StorageT* const    obj
,   T* const                    expected
,   const T                     desired
,   const memory_order          success
,   const memory_order          failure
) MGBASE_NOEXCEPT
{
    // Expand to the storage type.
    StorageT expected_tmp = cast_to_atomic_storage<StorageT>(*expected);
    
    const bool result = compare_exchange_weak_impl(
        obj
    ,   &expected_tmp
    ,   cast_to_atomic_storage<StorageT>(desired)
    ,   success
    ,   failure
    );
    
    // Shrink from the storage type.
    *expected = cast_from_atomic_storage<T>(expected_tmp);
    
    return result;
}

template <typename T, typename StorageT>
MGBASE_ALWAYS_INLINE bool compare_exchange_strong(
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

// Fetch and op
// Implemented on CAS

#define DEFINE_FETCH_OP(name, NAME, c_op) \
    template <typename StorageT, typename T> \
    MGBASE_ALWAYS_INLINE T fetch_##name(volatile StorageT* const obj, const T arg, const memory_order order) MGBASE_NOEXCEPT \
    { \
        while (true) { \
            /* Load and shrink the storage. */ \
            const T old = cast_from_atomic_storage<T>(*obj); \
            T expected = old; \
            const T desired = expected c_op arg; \
            if (MGBASE_LIKELY(compare_exchange_weak(obj, &expected, desired, order, order))) \
                return old; \
        } \
    }

MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OP)

#undef DEFINE_FETCH_OP

} // namespace detail

} // namespace mgbase

