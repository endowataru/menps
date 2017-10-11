
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

} // namespace untyped

template <typename T>
inline T* to_raw_pointer(const remote_ptr<T>& ptr) MGBASE_NOEXCEPT {
    return static_cast<T*>(untyped::to_raw_pointer(ptr.to_address()));
}

} // namespace rma
} // namespace mgcom

