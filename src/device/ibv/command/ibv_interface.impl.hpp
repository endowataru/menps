
#pragma once

#include <mgcom/rma/try_rma.hpp>

namespace mgcom {

namespace ibv {

// /*???*/ g_proxy;

} // namespace ibv

namespace rma {

namespace untyped {

MGBASE_WARN_UNUSED_RESULT bool try_remote_read_async(
    const process_id_t          src_proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_read_async(
        src_proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_write_async(
    const process_id_t          dest_proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_write_async(
        dest_proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   on_complete
    );
}

} // namespace untyped

MGBASE_WARN_UNUSED_RESULT
bool try_remote_atomic_read_async(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   atomic_default_t* const                     dest_ptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_atomic_read_async(
        src_proc
    ,   src_rptr
    ,   dest_ptr
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_remote_atomic_write_async(
    const process_id_t                  dest_proc
,   const remote_ptr<atomic_default_t>& dest_rptr
,   const atomic_default_t              value
,   const mgbase::operation&            on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_atomic_write_async(
        dest_proc
    ,   dest_rptr
    ,   value
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_remote_compare_and_swap_async(
    const process_id_t                  target_proc
,   const remote_ptr<atomic_default_t>& target_rptr
,   const atomic_default_t              expected
,   const atomic_default_t              desired
,   atomic_default_t* const             result_ptr
,   const mgbase::operation&            on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_compare_and_swap_async(
        target_proc
    ,   target_rptr
    ,   expected
    ,   desired
    ,   result_ptr
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT
bool try_remote_fetch_and_add_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      value
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_fetch_and_add_async(
        target_proc
    ,   target_rptr
    ,   value
    ,   result_ptr
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

