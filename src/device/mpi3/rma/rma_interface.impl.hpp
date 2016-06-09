
#pragma once

#include "rma.hpp"
#include "common/rma/rma.hpp"
#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace rma {

namespace mpi3 {

// Points to a global variable defined in another compilation unit.
//  /*???*/& g_queue;

} // namespace mpi3

namespace untyped {

bool try_remote_read_async(
    const process_id_t          src_proc
,   const remote_address&       src_raddr
,   const local_address&        dest_laddr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return mgcom::mpi3::g_queue.try_get(
        to_raw_pointer(dest_laddr)
    ,   static_cast<int>(src_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(src_raddr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

bool try_remote_write_async(
    const process_id_t          dest_proc
,   const remote_address&       dest_raddr
,   const local_address&        src_laddr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return mgcom::mpi3::g_queue.try_put(
        to_raw_pointer(src_laddr)
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(dest_raddr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

} // namespace untyped

bool try_remote_atomic_read_async(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   atomic_default_t* const                     dest_ptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::mpi3::g_queue.try_fetch_and_op<atomic_default_t>(
        0 // parameter "value" is unused
    ,   dest_ptr
    ,   static_cast<int>(src_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(src_rptr))
    ,   MPI_NO_OP
    ,   on_complete
    );
}

bool try_remote_atomic_write_async(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const atomic_default_t                      value
,   const mgbase::operation&                    on_complete
) {
    return mgcom::mpi3::g_queue.try_fetch_and_op<atomic_default_t>(
        value
    ,   MGBASE_NULLPTR
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(dest_rptr))
    ,   MPI_REPLACE
    ,   on_complete
    );
}

bool try_remote_compare_and_swap_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      expected_val
,   const atomic_default_t                      desired_val
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::mpi3::g_queue.try_compare_and_swap<atomic_default_t>(
        expected_val
    ,   desired_val
    ,   result_ptr
    ,   static_cast<int>(target_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(target_rptr))
    ,   on_complete
    );
}

bool try_remote_fetch_and_add_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      value
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    return mgcom::mpi3::g_queue.try_fetch_and_op<atomic_default_t>(
        value
    ,   result_ptr
    ,   static_cast<int>(target_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(target_rptr))
    ,   MPI_SUM
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

