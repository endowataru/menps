
#include "mpi_command_queue_base.hpp"
#include "device/mpi/mpi_call.hpp"

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

bool try_test(
    MPI_Request* const          request
,   bool* const                 success_result
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_test(
        request
    ,   success_result
    ,   on_complete
    );
}

bool try_testany(
    const int                   count
,   MPI_Request* const          first_request
,   int* const                  index_result
,   bool* const                 success_result
,   const mgbase::operation&    on_complete
) {
    return g_queue.try_testany(
        count
    ,   first_request
    ,   index_result
    ,   success_result
    ,   on_complete
    );
}

MPI_Comm comm_dup(const MPI_Comm comm)
{
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    MPI_Comm result;
    mpi_error::check(
        MPI_Comm_dup(comm, &result)
    );
    
    MGBASE_LOG_DEBUG("msg:Duplicated communicator.");
    
    return result;
}

void comm_set_name(const MPI_Comm comm, const char* const comm_name)
{
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    mpi_error::check(
        MPI_Comm_set_name(
            comm
        ,   const_cast<char*>(comm_name) // const_cast is required for old MPI libraries
        )
    );
    
    MGBASE_LOG_DEBUG("msg:Set communicator name.\tname:{}", comm_name);
}

void blocking_barrier()
{
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    MPI_Barrier(MPI_COMM_WORLD); // TODO: use another communicator
}

void mpi_lock::lock()
{
    g_queue.lock();
}

bool mpi_lock::try_lock()
{
    return g_queue.try_lock();
}

void mpi_lock::unlock()
{
    g_queue.unlock();
}

} // namespace mpi
} // namespace mgcom

