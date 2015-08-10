
#pragma once

#include <mgcom.hpp>

namespace mgcom {

/*inline local_region_address_t get_absolute_address(const local_address_t& addr) {
    return addr.region.local_address  + addr.offset;
}

inline local_region_address_t get_absolute_address(const remote_address_t& addr) {
    return addr.region.remote_address + addr.offset;
}*/

inline void notify(const notifier_t& notif) MGBASE_NOEXCEPT {
    switch (notif.operation) {
        case MGCOM_LOCAL_NO_OPERATION:
            // Do nothing.
            break;
        
        case MGCOM_LOCAL_ASSIGN_INT8:
            *static_cast<mgbase::uint8_t*>(notif.pointer) = static_cast<mgbase::uint8_t>(notif.value);
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

inline notifier_t make_notifier_no_operation() {
    notifier_t result = { MGCOM_LOCAL_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline notifier_t make_notifier_assign(bool* ptr, bool value) {
    notifier_t result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}

inline notifier_t make_notifier_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
inline notifier_t make_notifier_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

inline notifier_t make_notifier_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
inline notifier_t make_notifier_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

}

