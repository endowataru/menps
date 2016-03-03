
#pragma once

#include "mpi3_command_queue_base.hpp"
#include "device/mpi3/mpi3_call.hpp"

#include "device/mpi3/collective/collective.impl.hpp"

namespace mgcom {
namespace mpi3 {

// Points to a global variable defined in another compilation unit.
//  /**/ & g_queue;

bool try_get(
    void*                       dest_ptr
,   int                         src_rank
,   MPI_Aint                    src_index
,   int                         size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_get(
        dest_ptr
    ,   src_rank
    ,   src_index
    ,   size_in_bytes
    ,   on_complete
    );
}

bool try_put(
    const void*                 src_ptr
,   int                         dest_rank
,   MPI_Aint                    dest_index
,   int                         size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_put(
        src_ptr
    ,   dest_rank
    ,   dest_index
    ,   size_in_bytes
    ,   on_complete
    );
}

bool try_compare_and_swap(
    const void*                 expected_ptr
,   const void*                 desired_ptr
,   void*                       result_ptr
,   MPI_Datatype                datatype
,   int                         dest_rank
,   MPI_Aint                    dest_index
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_compare_and_swap(
        expected_ptr
    ,   desired_ptr
    ,   result_ptr
    ,   datatype
    ,   dest_rank
    ,   dest_index
    ,   on_complete
    );
}

bool try_fetch_and_op(
    const void*                 value_ptr
,   void*                       result_ptr
,   MPI_Datatype                datatype
,   int                         dest_rank
,   MPI_Aint                    dest_index
,   MPI_Op                      operation
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_fetch_and_op(
        value_ptr
    ,   result_ptr
    ,   datatype
    ,   dest_rank
    ,   dest_index
    ,   operation
    ,   on_complete
    );
}

} // namespace mpi3
} // namespace mgcom

