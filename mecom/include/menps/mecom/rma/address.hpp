
#pragma once

#include <menps/mecom/rma/common.hpp>
#include <menps/mecom/rma/address.h>

namespace menps {
namespace mecom {
namespace rma {

namespace /*unnamed*/ {

constexpr index_t registration_alignment = MECOM_REGISTRATION_ALIGNMENT;

constexpr index_t buffer_alignment       = MECOM_BUFFER_ALIGNMENT;

} // unnamed namespace

namespace untyped {

typedef mecom_rma_address_offset_t         address_offset_t;

typedef mecom_rma_region_key               region_key;
typedef mecom_rma_local_region             local_region;
typedef mecom_rma_remote_region            remote_region;

typedef mecom_rma_local_address            local_address;
typedef mecom_rma_remote_address           remote_address;

typedef mecom_rma_registered_buffer   registered_buffer;

inline region_key to_region_key(const local_region& region) noexcept {
    return region.key;
}

inline local_address to_address(const local_region& region) noexcept {
    local_address addr = { region, 0 };
    return addr;
}

inline remote_address to_address(const remote_region& region) noexcept {
    remote_address addr = { region, 0 };
    return addr;
}

inline local_address to_address(const registered_buffer& buffer) noexcept {
    return buffer.addr;
}

inline local_address advanced(const local_address& addr, const mefdn::ptrdiff_t diff) noexcept {
    local_address result = { addr.region, static_cast<index_t>(static_cast<mefdn::ptrdiff_t>(addr.offset) + diff) };
    return result;
}
inline remote_address advanced(const remote_address& addr, const mefdn::ptrdiff_t diff) noexcept {
    remote_address result = { addr.region, static_cast<index_t>(static_cast<mefdn::ptrdiff_t>(addr.offset) + diff) };
    return result;
}

inline void* to_raw_pointer(const local_region& region) noexcept {
    return region.key.pointer;
}

inline void* to_raw_pointer(const local_address& addr) noexcept {
    return static_cast<mefdn::uint8_t*>(to_raw_pointer(addr.region)) + addr.offset;
}

inline void* to_raw_pointer(const registered_buffer& buffer) noexcept {
    return to_raw_pointer(to_address(buffer));
}

} // namespace untyped

inline mefdn::uint64_t to_integer(const untyped::remote_address& addr) noexcept {
    return reinterpret_cast<mefdn::uint64_t>(
        static_cast<mefdn::uint8_t*>(addr.region.key.pointer) + addr.offset
    );
}
inline mefdn::uint64_t to_integer(const untyped::local_address& addr) noexcept {
    return reinterpret_cast<mefdn::uint64_t>(
        static_cast<mefdn::uint8_t*>(untyped::to_raw_pointer(addr.region)) + addr.offset
    );
}

} // namespace rma
} // namespace mecom
} // namespace menps

