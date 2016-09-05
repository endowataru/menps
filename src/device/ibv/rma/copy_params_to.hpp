
#pragma once

#include "device/ibv/command/params.hpp"
#include "address.hpp"

namespace mgcom {
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
    const rma::atomic_read_params<T>    from
#if 0
,   const rma::local_ptr<T>&            result_lptr
#endif
,   atomic_read_command* const          to
) {
    const auto src_raddr = from.src_rptr.to_address();
    
    to->raddr         = to_raddr(src_raddr);
    to->rkey          = to_rkey(src_raddr);
    to->dest_ptr      = from.dest_ptr;
    to->on_complete   = from.on_complete;
    
#if 0
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
#endif
}

template <typename T>
inline void copy_atomic_write_params_to(
    const rma::atomic_write_params<T>   from
#if 0
,   const rma::local_ptr<T>&            result_lptr
#endif
,   atomic_write_command* const         to
) {
    const auto dest_raddr = from.dest_rptr.to_address();
    
    to->raddr         = to_raddr(dest_raddr);
    to->rkey          = to_rkey(dest_raddr);
    to->value         = from.value;
    to->on_complete   = from.on_complete;
    
#if 0
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
#endif
}

template <typename T>
inline void copy_compare_and_swap_params_to(
    const rma::compare_and_swap_params<T>&  from
#if 0
,   const rma::local_ptr<T>&                result_lptr
#endif
,   compare_and_swap_command* const         to
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
    const rma::fetch_and_add_params<T>& from
#if 0
,   const rma::local_ptr<T>&             result_lptr
#endif
,   fetch_and_add_command* const        to
) {
    to->raddr       = to_raddr(from.target_rptr);
    to->rkey        = to_rkey(from.target_rptr);
    to->value       = from.value;
    to->result_ptr  = from.result_ptr;
    to->on_complete = from.on_complete;
}

} // namespace ibv
} // namespace mgcom

