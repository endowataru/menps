
#include "mpi_command_queue_base.hpp"
#include "device/mpi/mpi_call.hpp"
#include "common/command/comm_call.hpp"

namespace mgcom {
namespace mpi {

// Points to a global variable defined in another compilation unit.
extern mpi_command_queue_base& g_queue;

bool try_irecv(
    void* const                 buf
,   const int                   size_in_bytes
,   const int                   source_rank
,   const int                   tag
,   const MPI_Comm              comm
,   MPI_Status* const           status_result
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_irecv(
        buf
    ,   size_in_bytes
    ,   source_rank
    ,   tag
    ,   comm
    ,   status_result
    ,   on_complete
    );
}

bool try_isend(
    const void* const           buf
,   const int                   size_in_bytes
,   const int                   dest_rank
,   const int                   tag
,   const MPI_Comm              comm
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_isend(
        buf
    ,   size_in_bytes
    ,   dest_rank
    ,   tag
    ,   comm
    ,   on_complete
    );
}

bool try_irsend(
    const void* const           buf
,   const int                   size_in_bytes
,   const int                   dest_rank
,   const int                   tag
,   const MPI_Comm              comm
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_irsend(
        buf
    ,   size_in_bytes
    ,   dest_rank
    ,   tag
    ,   comm
    ,   on_complete
    );
}

} // namespace mpi

namespace detail {

bool try_comm_call(const mgbase::bound_function<void ()>& func)
{
    return mpi::g_queue.try_call(func);
}

} // namespace detail

} // namespace mgcom

