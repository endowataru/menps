
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

inline void* to_raw_pointer(const remote_region& region) MGBASE_NOEXCEPT {
    return region.key.pointer;
}

inline void* to_raw_pointer(const remote_address& addr) MGBASE_NOEXCEPT {
    return static_cast<mgbase::uint8_t*>(to_raw_pointer(addr.region)) + addr.offset;
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


} // namespace untyped

template <typename T>
inline T* to_raw_pointer(const remote_ptr<T>& ptr) MGBASE_NOEXCEPT {
    return static_cast<T*>(untyped::to_raw_pointer(ptr.to_address()));
}

} // namespace rma
} // namespace mgcom

