
#pragma once

#include <mgbase/operation.h>
#include <mgbase/threading/atomic.hpp>

namespace mgbase {

typedef mgbase_operation_code     operation_code;
typedef mgbase_operation          operation;

namespace /*unnamed*/ {

inline operation make_operation_no_operation() MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline operation make_operation_store_release(bool* ptr, bool value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT8, ptr, value };
    return result;
}
inline operation make_operation_store_release(mgbase::uint8_t* ptr, mgbase::uint8_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT8, ptr, value };
    return result;
}
inline operation make_operation_store_release(mgbase::uint16_t* ptr, mgbase::uint16_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT16, ptr, value };
    return result;
}
inline operation make_operation_store_release(mgbase::uint32_t* ptr, mgbase::uint32_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT32, ptr, value };
    return result;
}
inline operation make_operation_store_release(mgbase::uint64_t* ptr, mgbase::uint64_t value) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_STORE_RELEASE_INT64, ptr, value };
    return result;
}

inline operation make_operation_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
inline operation make_operation_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

inline operation make_operation_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
inline operation make_operation_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    operation result = { MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

inline MGBASE_ALWAYS_INLINE void execute(const operation& op) MGBASE_NOEXCEPT
{
    switch (op.code)
    {
        case MGBASE_OPERATION_STORE_RELEASE_INT8:
            *static_cast<mgbase::uint8_t*>(op.pointer) = static_cast<mgbase::uint8_t>(op.value);
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT16:
            *static_cast<mgbase::int16_t*>(op.pointer) = static_cast<mgbase::int16_t>(op.value);
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT32:
            *static_cast<mgbase::int32_t*>(op.pointer) = static_cast<mgbase::int32_t>(op.value);
            break;
        
        case MGBASE_OPERATION_STORE_RELEASE_INT64:
            *static_cast<mgbase::int64_t*>(op.pointer) = static_cast<mgbase::int64_t>(op.value);
            break;
        
        case MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32:
            static_cast<mgbase::atomic<mgbase::int32_t>*>(op.pointer)
                ->fetch_add(static_cast<mgbase::int32_t>(op.value), mgbase::memory_order_relaxed);
            break;
        
        case MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64:
            static_cast<mgbase::atomic<mgbase::int64_t>*>(op.pointer)
                ->fetch_add(static_cast<mgbase::int64_t>(op.value), mgbase::memory_order_relaxed);
            break;
        
        case MGBASE_OPERATION_NO_OPERATION:
            // Do nothing.
            break;
        
        default:
            MGBASE_UNREACHABLE();
            break;
    }
}

} // unnamed namespace

} // namespace mgbase

