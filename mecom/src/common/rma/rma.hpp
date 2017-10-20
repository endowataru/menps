
#pragma once

#include <menps/mecom/rma.hpp>
#include <menps/mecom/rma/pointer.hpp>
#include <menps/mecom/rma/to_raw.hpp>

namespace menps {
namespace mecom {
namespace rma {

namespace untyped {

inline region_key make_region_key(void* pointer, mefdn::uint64_t info) noexcept {
    region_key key = { pointer, info };
    return key;
}

inline local_region make_local_region(const region_key& key, mefdn::uint64_t info) noexcept {
    local_region region = { key, info };
    return region;
}

inline remote_region make_remote_region(const region_key& key, mefdn::uint64_t info) noexcept {
    remote_region region = { key, info };
    return region;
}


} // namespace untyped

} // namespace rma
} // namespace mecom
} // namespace menps

