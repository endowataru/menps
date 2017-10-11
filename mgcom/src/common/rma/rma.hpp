
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rma/pointer.hpp>
#include <mgcom/rma/to_raw.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

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

} // namespace rma
} // namespace mgcom

