
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
    process_id_t                                src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   const local_ptr<atomic_default_t>&          dest_lptr
,   const local_ptr<atomic_default_t>&          //buf_lptr
,   const mgbase::operation&                    on_complete
) {
    // TODO: Is this correct?
    return untyped::try_remote_read_async(
        src_proc
    ,   src_rptr.to_address()
    ,   dest_lptr.to_address()
    ,   sizeof(atomic_default_t)
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_write_async(
    process_id_t                                dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const local_ptr<const atomic_default_t>&    src_lptr
,   const local_ptr<atomic_default_t>&          //buf_lptr
,   const mgbase::operation&                    on_complete
) {
    // TODO: Is this correct?
    return untyped::try_remote_write_async(
        dest_proc
    ,   dest_rptr.to_address()
    ,   src_lptr.to_address()
    ,   sizeof(atomic_default_t)
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_compare_and_swap_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const local_ptr<const atomic_default_t>&    expected_lptr
,   const local_ptr<const atomic_default_t>&    desired_lptr
,   const local_ptr<atomic_default_t>&          result_lptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_compare_and_swap_async(
        target_proc
    ,   target_rptr
    ,   expected_lptr
    ,   desired_lptr
    ,   result_lptr
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_fetch_and_add_async(
    process_id_t                                target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const local_ptr<const atomic_default_t>&    value_lptr
,   const local_ptr<atomic_default_t>&          result_lptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::ibv::g_proxy.try_remote_fetch_and_add_async(
        target_proc
    ,   target_rptr
    ,   value_lptr
    ,   result_lptr
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

