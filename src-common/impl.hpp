
#pragma once

#include <mgcom.hpp>

namespace mgcom {

namespace rma {

namespace {

inline void* to_pointer(const remote_region& region) MGBASE_NOEXCEPT {
    return region.key.pointer;
}

inline void* to_pointer(const remote_address& addr) MGBASE_NOEXCEPT {
    return static_cast<mgbase::uint8_t*>(to_pointer(addr.region)) + addr.offset;
}

inline region_key make_region_key(void* pointer, mgbase::uint64_t info) MGBASE_NOEXCEPT {
    region_key key = { pointer, info };
    return key;
}

inline local_region make_local_region(const region_key& key, mgbase::uint64_t info) MGBASE_NOEXCEPT {
    local_region region = { key, info };
    return region;
}

inline remote_region make_remote_region(const region_key& key, mgbase::uint64_t info) MGBASE_NOEXCEPT {
    remote_region region = { key, info };
    return region;
}

}

}

namespace {

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

