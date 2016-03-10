
#pragma once

#include <mgcom/rma/address.hpp>
#include "device/ibv/native/verbs.hpp"

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

MGBASE_ALWAYS_INLINE mgbase::uint64_t to_raddr(const rma::untyped::remote_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(addr.region.key.pointer) + addr.offset;
}

MGBASE_ALWAYS_INLINE mgbase::uint32_t to_rkey(const rma::untyped::remote_address& addr) MGBASE_NOEXCEPT {
    return addr.region.key.info;
}

MGBASE_ALWAYS_INLINE ibv_mr* to_mr(const rma::untyped::local_region& region) MGBASE_NOEXCEPT {
    return reinterpret_cast<ibv_mr*>(region.info);
}

MGBASE_ALWAYS_INLINE mgbase::uint64_t to_laddr(const rma::untyped::local_address& addr) MGBASE_NOEXCEPT {
    const ibv_mr* const mr = to_mr(addr.region);
    return reinterpret_cast<mgbase::uint64_t>(mr->addr) + addr.offset;
}

MGBASE_ALWAYS_INLINE mgbase::uint32_t to_lkey(const rma::untyped::local_address& addr) MGBASE_NOEXCEPT {
    const ibv_mr* const mr = to_mr(addr.region);
    return mr->lkey;
}

} // unnamed namespace

} // namespace ibv
} // namespace mgcom

