
#pragma once

#include <mgcom.hpp>
#include <mgbase/threading/atomic.hpp>
#include "notifier.h"

namespace mgcom {

typedef mgcom_local_operation          local_operation;
typedef mgcom_local_notifier           local_notifier;

typedef mgcom_remote_operation         remote_operation;

namespace /*unnamed*/ {

inline local_notifier make_notifier_no_operation() MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline local_notifier make_notifier_assign(bool* ptr, bool value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint8_t* ptr, mgbase::uint8_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint16_t* ptr, mgbase::uint16_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT16, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint32_t* ptr, mgbase::uint32_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT32, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint64_t* ptr, mgbase::uint64_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT64, ptr, value };
    return result;
}

inline local_notifier make_notifier_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
inline local_notifier make_notifier_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

inline local_notifier make_notifier_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
inline local_notifier make_notifier_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

template <typename CB>
inline local_notifier make_notifier_finished(CB& cb) {
    return make_notifier_assign(&cb.common.finished, true);
}

inline void notify(const local_notifier& notif) MGBASE_NOEXCEPT {
    switch (notif.operation) {
        case MGCOM_LOCAL_NO_OPERATION:
            // Do nothing.
            break;
        
        case MGCOM_LOCAL_ASSIGN_INT8:
            *static_cast<mgbase::uint8_t*>(notif.pointer) = static_cast<mgbase::uint8_t>(notif.value);
            break;
        
        case MGCOM_LOCAL_ASSIGN_INT16:
            *static_cast<mgbase::int16_t*>(notif.pointer) = static_cast<mgbase::int16_t>(notif.value);
            break;
        
        case MGCOM_LOCAL_ASSIGN_INT32:
            *static_cast<mgbase::int32_t*>(notif.pointer) = static_cast<mgbase::int32_t>(notif.value);
            break;
        
        case MGCOM_LOCAL_ASSIGN_INT64:
            *static_cast<mgbase::int64_t*>(notif.pointer) = static_cast<mgbase::int64_t>(notif.value);
            break;
        
        case MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32:
            static_cast<mgbase::atomic<mgbase::int32_t>*>(notif.pointer)
                ->fetch_add(static_cast<mgbase::int32_t>(notif.value), mgbase::memory_order_relaxed);
            break;
        
        case MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64:
            static_cast<mgbase::atomic<mgbase::int64_t>*>(notif.pointer)
                ->fetch_add(static_cast<mgbase::int64_t>(notif.value), mgbase::memory_order_relaxed);
            break;
    }
}

}

}

