
#pragma once

#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace mpi3 {

bool try_get(
    void*                       dest_ptr
,   int                         src_rank
,   MPI_Aint                    src_index
,   int                         size_in_bytes
,   const mgbase::operation&    on_complete
);

bool try_put(
    const void*                 src_ptr
,   int                         dest_rank
,   MPI_Aint                    dest_index
,   int                         size_in_bytes
,   const mgbase::operation&    on_complete
);

bool try_compare_and_swap(
    const void*                 expected_ptr
,   const void*                 desired_ptr
,   void*                       result_ptr
,   MPI_Datatype                datatype
,   int                         dest_rank
,   MPI_Aint                    dest_index
,   const mgbase::operation&    on_complete
);

bool try_fetch_and_op(
    const void*                 value_ptr
,   void*                       result_ptr
,   MPI_Datatype                datatype
,   int                         dest_rank
,   MPI_Aint                    dest_index
,   MPI_Op                      operation
,   const mgbase::operation&    on_complete
);

} // namespace mpi3
} // namespace mgcom

