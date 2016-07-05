
#pragma once

#include "device/ibv/command/params.hpp"
#include "address.hpp"

namespace mgcom {
namespace ibv {

inline void copy_read_params_to(
    const rma::untyped::read_params&    from
,   read_params* const                  to
) {
    // to->wr_id is unset
    to->raddr         = to_raddr(from.src_raddr);
    to->rkey          = to_rkey(from.src_raddr);
    to->laddr         = to_laddr(from.dest_laddr);
    to->lkey          = to_lkey(from.dest_laddr);
    to->size_in_bytes = from.size_in_bytes;
}

inline void copy_write_params_to(
    const rma::untyped::write_params&   from
,   write_params* const                 to
) {
    // to->wr_id is unset
    to->raddr         = to_raddr(from.dest_raddr);
    to->rkey          = to_rkey(from.dest_raddr);
    to->laddr         = to_laddr(from.src_laddr);
    to->lkey          = to_lkey(from.src_laddr);
    to->size_in_bytes = from.size_in_bytes;
}

template <typename T>
inline void copy_atomic_read_params_to(
    const rma::atomic_read_params<T>    from
,   const rma::local_ptr<T>&            result_lptr
,   read_params* const                  to
) {
    // TODO: Atomicity of ordinary read
    copy_read_params_to(
        rma::untyped::read_params{
            from.src_proc // Note: unused
        ,   from.src_rptr.to_address()
        ,   result_lptr.to_address()
        ,   sizeof(rma::atomic_default_t)
        ,   from.on_complete // Note: unused
        }
    ,   to
    );
}

template <typename T>
inline void copy_atomic_write_params_to(
    const rma::atomic_write_params<T>   from
,   const rma::local_ptr<T>&            result_lptr
,   write_params* const                 to
) {
    // TODO: Atomicity of ordinary read
    copy_write_params_to(
        rma::untyped::write_params{
            from.dest_proc // Note: unused
        ,   from.dest_rptr.to_address()
        ,   result_lptr.to_address()
        ,   sizeof(rma::atomic_default_t)
        ,   from.on_complete // Note: unused
        }
    ,   to
    );
}

template <typename T>
inline void copy_compare_and_swap_params_to(
    const rma::compare_and_swap_params<T>&  from
,   const rma::local_ptr<T>&                result_lptr
,   compare_and_swap_params* const          to
) {
    // to->wr_id is unset
    to->raddr       = to_raddr(from.target_rptr);
    to->rkey        = to_rkey(from.target_rptr);
    to->laddr       = to_laddr(result_lptr);
    to->lkey        = to_lkey(result_lptr);
    to->expected    = from.expected;
    to->desired     = from.desired;
}

template <typename T>
inline void copy_fetch_and_add_params_to(
    const rma::fetch_and_add_params<T>&  from
,   const rma::local_ptr<T>&             result_lptr
,   fetch_and_add_params* const          to
) {
    // to->wr_id is unset
    to->raddr       = to_raddr(from.target_rptr);
    to->rkey        = to_rkey(from.target_rptr);
    to->laddr       = to_laddr(result_lptr);
    to->lkey        = to_lkey(result_lptr);
    to->value       = from.value;
}

} // namespace ibv
} // namespace mgcom

