
#pragma once

#include <mgcom/rma/try_rma.hpp>

namespace mgcom {

namespace ibv {

// /*???*/ g_proxy;

namespace rma {

namespace untyped {

MGBASE_WARN_UNUSED_RESULT
bool try_read_async(const untyped::read_params& params) {
    return mgcom::ibv::g_proxy.try_remote_read_async(
        params.src_proc
    ,   params.src_raddr
    ,   params.dest_laddr
    ,   params.size_in_bytes
    ,   params.on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_write_async(const untyped::write_params& params) {
    return mgcom::ibv::g_proxy.try_remote_write_async(
        params.dest_proc
    ,   params.dest_raddr
    ,   params.src_laddr
    ,   params.size_in_bytes
    ,   params.on_complete
    );
}

} // namespace untyped

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy.try_remote_atomic_read_async(
        params.src_proc
    ,   params.src_rptr
    ,   params.dest_ptr
    ,   params.on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy.try_remote_atomic_write_async(
        params.dest_proc
    ,   params.dest_rptr
    ,   params.value
    ,   params.on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy.try_remote_compare_and_swap_async(
        params.target_proc
    ,   params.target_rptr
    ,   params.expected
    ,   params.desired
    ,   params.result_ptr
    ,   params.on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params) {
    return mgcom::ibv::g_proxy.try_remote_fetch_and_add_async(
        params.target_proc
    ,   params.target_rptr
    ,   params.value
    ,   params.result_ptr
    ,   params.on_complete
    );
}

} // namespace rma
} // namespace ibv
} // namespace mgcom

