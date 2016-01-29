
#pragma once

#include <mgbase/operation.h>
#include <mgbase/atomic.hpp>

namespace mgbase {

typedef mgbase_operation_code     operation_code;
typedef mgbase_operation          operation;

namespace detail {

namespace /*unnamed*/ {

template <typename T>
MGBASE_ALWAYS_INLINE void execute_store_release(const operation& op) MGBASE_NOEXCEPT
{
    T* const target = static_cast<T*>(op.pointer);
    const T value = static_cast<T>(op.value);
    mgbase::atomic_store_explicit(target, value, mgbase::memory_order_release);
}

template <typename T>
MGBASE_ALWAYS_INLINE void execute_fetch_add(const operation& op) MGBASE_NOEXCEPT
{
    T* const target = static_cast<T*>(op.pointer);
    const T value = static_cast<T>(op.value);
    //mgbase::atomic_fetch_add_explicit(target, value, mgbase::memory_order_release);
    mgbase::atomic_fetch_add(target, value);
}

} // unnamed namespace

} // namespace detail

namespace /*unnamed*/ {

MGBASE_ALWAYS_INLINE void execute(const operation& op) MGBASE_NOEXCEPT
{
    switch (op.code)
    {
        case MGBASE_OPERATION_STORE_RELEASE_INT8:
            detail::execute_store_release<mgbase::uint8_t>(op);
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT16:
            detail::execute_store_release<mgbase::uint16_t>(op);
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT32:
            detail::execute_store_release<mgbase::uint32_t>(op);
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT64:
            detail::execute_store_release<mgbase::uint64_t>(op);
            break;
        
        case MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32:
            detail::execute_fetch_add<mgbase::uint32_t>(op);
            break;
        
        case MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64:
            detail::execute_fetch_add<mgbase::uint64_t>(op);
            break;
        
        case MGBASE_OPERATION_NO_OPERATION:
            // Do nothing.
            break;
        
        default:
            MGBASE_UNREACHABLE();
            break;
    }
}


MGBASE_ALWAYS_INLINE operation make_no_operation() MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

MGBASE_ALWAYS_INLINE operation make_operation_store_release(bool* ptr, bool value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT8, ptr, value };
    return result;
}
MGBASE_ALWAYS_INLINE operation make_operation_store_release(mgbase::uint8_t* ptr, mgbase::uint8_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT8, ptr, value };
    return result;
}
MGBASE_ALWAYS_INLINE operation make_operation_store_release(mgbase::uint16_t* ptr, mgbase::uint16_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT16, ptr, value };
    return result;
}
MGBASE_ALWAYS_INLINE operation make_operation_store_release(mgbase::uint32_t* ptr, mgbase::uint32_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT32, ptr, value };
    return result;
}
MGBASE_ALWAYS_INLINE operation make_operation_store_release(mgbase::uint64_t* ptr, mgbase::uint64_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT64, ptr, value };
    return result;
}

MGBASE_ALWAYS_INLINE operation make_operation_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
MGBASE_ALWAYS_INLINE operation make_operation_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

MGBASE_ALWAYS_INLINE operation make_operation_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
MGBASE_ALWAYS_INLINE operation make_operation_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

} // unnamed namespace

} // namespace mgbase

