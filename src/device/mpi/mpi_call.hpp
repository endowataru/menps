
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/operation.hpp>
#include <mpi.h>

namespace mgcom {
namespace mpi {

// try_*

MGBASE_WARN_UNUSED_RESULT bool try_irecv(
    void*                       buf
,   int                         size_in_bytes
,   int                         source_rank
,   int                         tag
,   MPI_Comm                    comm
,   MPI_Status*                 status_result
,   const mgbase::operation&    on_complete
);

MGBASE_WARN_UNUSED_RESULT bool try_isend(
    const void*                 buf
,   int                         size_in_bytes
,   int                         dest_rank
,   int                         tag
,   MPI_Comm                    comm
,   const mgbase::operation&    on_complete
);

MGBASE_WARN_UNUSED_RESULT bool try_irsend(
    const void*                 buf
,   int                         size_in_bytes
,   int                         dest_rank
,   int                         tag
,   MPI_Comm                    comm
,   const mgbase::operation&    on_complete
);

MGBASE_WARN_UNUSED_RESULT bool try_cancel(
    MPI_Request*                request
,   const mgbase::operation&    on_complete
);

MPI_Comm comm_dup(MPI_Comm comm);

void comm_set_name(MPI_Comm comm, const char* comm_name);

/*
 * Important: This function blocks the communication thread.
 */
void blocking_barrier();

namespace /*unnamed*/ {

// ordinary non-blocking functions

/*
 * Although these functions will be blocked by other producers,
 * calling this function will not block forever.
 */

MGBASE_ALWAYS_INLINE void irecv(
    void*                       buf
,   int                         size_in_bytes
,   int                         source_rank
,   int                         tag
,   MPI_Comm                    comm
,   MPI_Status* const           status_result
,   const mgbase::operation&    on_complete
) {
    while (!try_irecv(
        buf
    ,   size_in_bytes
    ,   source_rank
    ,   tag
    ,   comm
    ,   status_result
    ,   on_complete
    ))
    { }
}

MGBASE_ALWAYS_INLINE void isend(
    const void* const           buf
,   const int                   size_in_bytes
,   const int                   dest_rank
,   const int                   tag
,   const MPI_Comm              comm
,   const mgbase::operation&    on_complete
) {
    while (!try_isend(
        buf
    ,   size_in_bytes
    ,   dest_rank
    ,   tag
    ,   comm
    ,   on_complete
    ))
    { }
}

MGBASE_ALWAYS_INLINE void irsend(
    const void* const           buf
,   const int                   size_in_bytes
,   const int                   dest_rank
,   const int                   tag
,   const MPI_Comm              comm
,   const mgbase::operation&    on_complete
) {
    while (!try_irsend(
        buf
    ,   size_in_bytes
    ,   dest_rank
    ,   tag
    ,   comm
    ,   on_complete
    ))
    { }
}

/*MGBASE_ALWAYS_INLINE void send(
    const void* const   buf
,   const int           size_in_bytes
,   const int           dest_rank
,   const int           tag
,   const MPI_Comm      comm
) {
    const MPI_Request req = isend(buf, size_in_bytes, dest_rank, tag, comm);
    
    complete(req, MPI_STATUS_IGNORE);
}*/

} // unnamed namespace




} // namespace mpi
} // namespace mgcom

