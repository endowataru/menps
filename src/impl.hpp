
#pragma once

#include <mgcom.hpp>
#include <mgbase/stdint.hpp>

namespace mgcom {

inline local_region_address_t get_absolute_address(const local_address_t& addr) {
    return addr.region.local_address  + addr.offset;
}

inline local_region_address_t get_absolute_address(const remote_address_t& addr) {
    return addr.region.remote_address + addr.offset;
}

inline void notify(const notifier_t& notif) MGBASE_NOEXCEPT {
    switch (notif.operation) {
        case MGCOM_LOCAL_ASSIGN_INT64:
            *static_cast<mgbase::int64_t*>(notif.pointer) = static_cast<mgbase::int64_t>(notif.value);
            break;
        case MGCOM_LOCAL_FAA_INT64:
            // FIXME
            break;
    }
}

}

