
#pragma once

#include <mgcom/rma/common.hpp>
#include <mgcom/rma/address.h>

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

MGBASE_UNUSED MGBASE_CONSTEXPR index_t registration_alignment = MGCOM_REGISTRATION_ALIGNMENT;

MGBASE_UNUSED MGBASE_CONSTEXPR index_t buffer_alignment       = MGCOM_BUFFER_ALIGNMENT;

} // unnamed namespace

namespace untyped {

typedef mgcom_rma_address_offset_t         address_offset_t;

typedef mgcom_rma_region_key               region_key;
typedef mgcom_rma_local_region             local_region;
typedef mgcom_rma_remote_region            remote_region;

typedef mgcom_rma_local_address            local_address;
typedef mgcom_rma_remote_address           remote_address;

typedef mgcom_rma_registered_buffer   registered_buffer;

/**
 * Allocate a registered buffer from the buffer pool.
 */
registered_buffer allocate(index_t size_in_bytes);

/**
 * Deallocate a registered buffer allocated from the buffer pool.
 */
void deallocate(const registered_buffer& buffer);


inline region_key to_region_key(const local_region& region) MGBASE_NOEXCEPT {
    return region.key;
}

inline local_address to_address(const local_region& region) MGBASE_NOEXCEPT {
    local_address addr = { region, 0 };
    return addr;
}

inline remote_address to_address(const remote_region& region) MGBASE_NOEXCEPT {
    remote_address addr = { region, 0 };
    return addr;
}

inline local_address to_address(const registered_buffer& buffer) MGBASE_NOEXCEPT {
    return buffer.addr;
}

inline local_address advanced(const local_address& addr, const mgbase::ptrdiff_t diff) MGBASE_NOEXCEPT {
    local_address result = { addr.region, static_cast<index_t>(static_cast<mgbase::ptrdiff_t>(addr.offset) + diff) };
    return result;
}
inline remote_address advanced(const remote_address& addr, const mgbase::ptrdiff_t diff) MGBASE_NOEXCEPT {
    remote_address result = { addr.region, static_cast<index_t>(static_cast<mgbase::ptrdiff_t>(addr.offset) + diff) };
    return result;
}

inline void* to_raw_pointer(const local_region& region) MGBASE_NOEXCEPT {
    return region.key.pointer;
}

inline void* to_raw_pointer(const local_address& addr) MGBASE_NOEXCEPT {
    return static_cast<mgbase::uint8_t*>(to_raw_pointer(addr.region)) + addr.offset;
}

inline void* to_raw_pointer(const registered_buffer& buffer) MGBASE_NOEXCEPT {
    return to_raw_pointer(to_address(buffer));
}

} // namespace untyped

inline mgbase::uint64_t to_integer(const untyped::remote_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(
        static_cast<mgbase::uint8_t*>(addr.region.key.pointer) + addr.offset
    );
}
inline mgbase::uint64_t to_integer(const untyped::local_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(
        static_cast<mgbase::uint8_t*>(untyped::to_raw_pointer(addr.region)) + addr.offset
    );
}

} // namespace rma
} // namespace mgcom

