
#pragma once

#include <menps/mecom/rma.hpp>
#include <menps/mecom/rma/pointer.hpp>

namespace menps {
namespace mecom {
namespace rma {

namespace untyped {

inline void* to_raw_pointer(const remote_region& region) noexcept {
    return region.key.pointer;
}

inline void* to_raw_pointer(const remote_address& addr) noexcept {
    return static_cast<mefdn::uint8_t*>(to_raw_pointer(addr.region)) + addr.offset;
}

} // namespace untyped

template <typename T>
inline T* to_raw_pointer(const remote_ptr<T>& ptr) noexcept {
    return static_cast<T*>(untyped::to_raw_pointer(ptr.to_address()));
}

} // namespace rma
} // namespace mecom
} // namespace menps

