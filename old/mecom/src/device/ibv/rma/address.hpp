
#pragma once

#include <menps/mecom/rma/pointer.hpp>
#include <menps/medev/ibv/verbs.hpp>

namespace menps {
namespace mecom {
namespace ibv {

inline mefdn::uint64_t to_raddr(const rma::untyped::remote_address& addr) noexcept {
    return reinterpret_cast<mefdn::uint64_t>(addr.region.key.pointer) + addr.offset;
}

template <typename T>
inline mefdn::uint64_t to_raddr(const rma::remote_ptr<T>& rptr) noexcept {
    return to_raddr(rptr.to_address());
}

inline mefdn::uint32_t to_rkey(const rma::untyped::remote_address& addr) noexcept {
    return static_cast<mefdn::uint32_t>(addr.region.key.info);
}

template <typename T>
inline mefdn::uint32_t to_rkey(const rma::remote_ptr<T>& rptr) noexcept {
    return to_rkey(rptr.to_address());
}

inline ibv_mr* to_mr(const rma::untyped::local_region& region) noexcept {
    return reinterpret_cast<ibv_mr*>(region.info);
}

inline mefdn::uint64_t to_laddr(const rma::untyped::local_address& addr) noexcept {
    const ibv_mr* const mr = to_mr(addr.region);
    return reinterpret_cast<mefdn::uint64_t>(mr->addr) + addr.offset;
}

template <typename T>
inline mefdn::uint64_t to_laddr(const rma::local_ptr<T>& lptr) noexcept {
    return to_laddr(lptr.to_address());
}

inline mefdn::uint32_t to_lkey(const rma::untyped::local_address& addr) noexcept {
    const ibv_mr* const mr = to_mr(addr.region);
    return mr->lkey;
}

template <typename T>
inline mefdn::uint32_t to_lkey(const rma::local_ptr<T>& lptr) noexcept {
    return to_lkey(lptr.to_address());
}

} // namespace ibv
} // namespace mecom
} // namespace menps

