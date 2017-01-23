
#pragma once

#include <mgcom/rma/pointer.hpp>
#include <mgdev/ibv/verbs.hpp>

namespace mgcom {
namespace ibv {

inline mgbase::uint64_t to_raddr(const rma::untyped::remote_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(addr.region.key.pointer) + addr.offset;
}

template <typename T>
inline mgbase::uint64_t to_raddr(const rma::remote_ptr<T>& rptr) MGBASE_NOEXCEPT {
    return to_raddr(rptr.to_address());
}

inline mgbase::uint32_t to_rkey(const rma::untyped::remote_address& addr) MGBASE_NOEXCEPT {
    return static_cast<mgbase::uint32_t>(addr.region.key.info);
}

template <typename T>
inline mgbase::uint32_t to_rkey(const rma::remote_ptr<T>& rptr) MGBASE_NOEXCEPT {
    return to_rkey(rptr.to_address());
}

inline ibv_mr* to_mr(const rma::untyped::local_region& region) MGBASE_NOEXCEPT {
    return reinterpret_cast<ibv_mr*>(region.info);
}

inline mgbase::uint64_t to_laddr(const rma::untyped::local_address& addr) MGBASE_NOEXCEPT {
    const ibv_mr* const mr = to_mr(addr.region);
    return reinterpret_cast<mgbase::uint64_t>(mr->addr) + addr.offset;
}

template <typename T>
inline mgbase::uint64_t to_laddr(const rma::local_ptr<T>& lptr) MGBASE_NOEXCEPT {
    return to_laddr(lptr.to_address());
}

inline mgbase::uint32_t to_lkey(const rma::untyped::local_address& addr) MGBASE_NOEXCEPT {
    const ibv_mr* const mr = to_mr(addr.region);
    return mr->lkey;
}

template <typename T>
inline mgbase::uint32_t to_lkey(const rma::local_ptr<T>& lptr) MGBASE_NOEXCEPT {
    return to_lkey(lptr.to_address());
}

} // namespace ibv
} // namespace mgcom

