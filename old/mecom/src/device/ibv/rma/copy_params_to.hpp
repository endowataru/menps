
#pragma once

#include "device/ibv/command/params.hpp"
#include "address.hpp"

namespace menps {
namespace mecom {
namespace ibv {

inline void copy_read_params_to(
    const rma::untyped::read_params&    from
,   read_command* const                 to
) {
    to->raddr         = to_raddr(from.src_raddr);
    to->rkey          = to_rkey(from.src_raddr);
    to->laddr         = to_laddr(from.dest_laddr);
    to->lkey          = to_lkey(from.dest_laddr);
    to->size_in_bytes = from.size_in_bytes;
    to->on_complete   = from.on_complete;
}

inline void copy_write_params_to(
    const rma::untyped::write_params&   from
,   write_command* const                to
) {
    to->raddr         = to_raddr(from.dest_raddr);
    to->rkey          = to_rkey(from.dest_raddr);
    to->laddr         = to_laddr(from.src_laddr);
    to->lkey          = to_lkey(from.src_laddr);
    to->size_in_bytes = from.size_in_bytes;
    to->on_complete   = from.on_complete;
}

template <typename T>
inline void copy_atomic_read_params_to(
    const rma::async_atomic_read_params<T>  from
,   atomic_read_command* const              to
) {
    // TODO: Atomicity of ordinary read
    
    const auto src_raddr = from.src_rptr.to_address();
    
    to->raddr         = to_raddr(src_raddr);
    to->rkey          = to_rkey(src_raddr);
    to->dest_ptr      = from.dest_ptr;
    to->on_complete   = from.on_complete;
}

template <typename T>
inline void copy_atomic_write_params_to(
    const rma::async_atomic_write_params<T> from
,   atomic_write_command* const             to
) {
    // TODO: Atomicity of ordinary write
    
    const auto dest_raddr = from.dest_rptr.to_address();
    
    to->raddr         = to_raddr(dest_raddr);
    to->rkey          = to_rkey(dest_raddr);
    to->value         = from.value;
    to->on_complete   = from.on_complete;
}

template <typename T>
inline void copy_compare_and_swap_params_to(
    const rma::async_compare_and_swap_params<T>&    from
,   compare_and_swap_command* const                 to
) {
    to->raddr       = to_raddr(from.target_rptr);
    to->rkey        = to_rkey(from.target_rptr);
    to->expected    = from.expected;
    to->desired     = from.desired;
    to->result_ptr  = from.result_ptr;
    to->on_complete = from.on_complete;
}

template <typename T>
inline void copy_fetch_and_add_params_to(
    const rma::async_fetch_and_add_params<T>&       from
,   fetch_and_add_command* const                    to
) {
    to->raddr       = to_raddr(from.target_rptr);
    to->rkey        = to_rkey(from.target_rptr);
    to->value       = from.value;
    to->result_ptr  = from.result_ptr;
    to->on_complete = from.on_complete;
}

} // namespace ibv
} // namespace mecom
} // namespace menps

