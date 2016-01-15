
#include "collective.ipp"

namespace mgcom {
namespace collective {

mgbase::deferred<void> barrier_nb(barrier_cb& cb)
{
    return detail::barrier_handlers::start(cb);
}

namespace untyped {

mgbase::deferred<void> broadcast(broadcast_cb& cb)
{
    return detail::broadcast_handlers::start(cb);
}

mgbase::deferred<void> allgather(allgather_cb& cb)
{
    return detail::allgather_handlers::start(cb);
}

} // namespace untyped

#if 0

namespace untyped {

void broadcast(process_id_t root, void* ptr, index_t number_of_bytes)
{
    MGBASE_LOG_DEBUG("msg:Started broadcast.");
    
    MPI_Request request;
    mpi_error::check(
        MPI_Ibcast(
            ptr
        ,   number_of_bytes
        ,   MPI_BYTE
        ,   static_cast<int>(root)
        ,   MPI_COMM_WORLD // TODO
        ,   &request
        )
    );
    
    while (true) {
        int flag;
        MPI_Status status;
        mpi_error::check(
            MPI_Test(&request, &flag, &status)
        );
        
        if (flag)
            break;
        
        mgbase::control::yield();
    }
    
    MGBASE_LOG_DEBUG("msg:Finished broadcast.");
}

void allgather(const void* src, void* dest, index_t number_of_bytes)
{
    MGBASE_LOG_DEBUG("msg:Started allgather.");
    
    MPI_Request request;
    mpi_error::check(
        MPI_Iallgather(
            src
        ,   number_of_bytes
        ,   MPI_BYTE
        ,   dest
        ,   number_of_bytes
        ,   MPI_BYTE
        ,   MPI_COMM_WORLD // TODO
        ,   &request
        )
    );
    
    while (true) {
        int flag;
        MPI_Status status;
        mpi_error::check(
            MPI_Test(&request, &flag, &status)
        );
        
        if (flag)
            break;
        
        mgbase::control::yield();
    }
    
    MGBASE_LOG_DEBUG("msg:Finished allgather.");
}

} // namespace untyped

void barrier()
{
    MGBASE_LOG_DEBUG("msg:Started barrier.");
    
    MPI_Request request;
    mpi_error::check(
        MPI_Ibarrier(
            MPI_COMM_WORLD // TODO
        ,   &request
        )
    );
    
    while (true) {
        int flag;
        MPI_Status status;
        mpi_error::check(
            MPI_Test(&request, &flag, &status)
        );
        
        if (flag)
            break;
        
        mgbase::control::yield();
    }
    
    MGBASE_LOG_DEBUG("msg:Finished barrier.");
}

#endif

} // namespace collective
} // namespace mgcom

