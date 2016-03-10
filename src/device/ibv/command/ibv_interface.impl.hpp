
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

MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_read_async(
    const process_id_t                              target_proc
,   const remote_pointer<const atomic_default_t>&   remote_ptr
,   const local_pointer<atomic_default_t>&          local_ptr
,   const local_pointer<atomic_default_t>&          /*buf_ptr*/
,   const mgbase::operation&                        on_complete
) {
    // TODO: Is this correct?
    return untyped::try_remote_read_async(
        target_proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   sizeof(atomic_default_t)
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_write_async(
    const process_id_t                              target_proc
,   const remote_pointer<atomic_default_t>&         remote_ptr
,   const local_pointer<const atomic_default_t>&    local_ptr
,   const local_pointer<atomic_default_t>&          /*buf_ptr*/
,   const mgbase::operation&                        on_complete
) {
    // TODO: Is this correct?
    return untyped::try_remote_write_async(
        target_proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   sizeof(atomic_default_t)
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_compare_and_swap_async(
    const process_id_t                              target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    expected_ptr
,   const local_pointer<const atomic_default_t>&    desired_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_compare_and_swap_async(
        target_proc
    ,   target_ptr
    ,   expected_ptr
    ,   desired_ptr
    ,   result_ptr
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_fetch_and_add_async(
    const process_id_t                              target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    value_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_fetch_and_add_async(
        target_proc
    ,   target_ptr
    ,   value_ptr
    ,   result_ptr
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

