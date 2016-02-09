
#include "mpi_call.hpp"
#include "mpi_base.hpp"

namespace mgcom {
namespace mpi {

void native_barrier()
{
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    MGBASE_LOG_DEBUG("msg:Executing MPI_Barrier.");
    
    mpi_error::check(
        MPI_Barrier(
            MPI_COMM_WORLD // TODO
        )
    );
}

namespace untyped {

void native_broadcast(
    const process_id_t  root
,   void* const         ptr
,   const index_t       number_of_bytes
) {
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    MGBASE_LOG_DEBUG(
        "msg:Executing MPI_Bcast.\t"
        "root:{}\tptr:{:x}\tnumber_of_bytes:{}"
    ,   root
    ,   reinterpret_cast<mgbase::intptr_t>(ptr)
    ,   number_of_bytes
    );
    
    mpi_error::check(
        MPI_Bcast(
            ptr // TODO
        ,   static_cast<int>(number_of_bytes)
        ,   MPI_BYTE
        ,   static_cast<int>(root)
        ,   MPI_COMM_WORLD // TODO
        )
    );
}

void native_allgather(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    MGBASE_LOG_DEBUG(
        "msg:Executing MPI_Allgather.\t"
        "src:{:x}\tdest:{:x}\tnumber_of_bytes:{}"
    ,   reinterpret_cast<mgbase::intptr_t>(src)
    ,   reinterpret_cast<mgbase::intptr_t>(dest)
    ,   number_of_bytes
    );
    
    mpi_error::check(
        MPI_Allgather(
            const_cast<void*>(src) // TODO: Only old versions of OpenMPI require
        ,   static_cast<int>(number_of_bytes)
        ,   MPI_BYTE
        ,   dest
        ,   static_cast<int>(number_of_bytes)
        ,   MPI_BYTE
        ,   MPI_COMM_WORLD // TODO
        )
    );
}

void native_alltoall(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
    
    MGBASE_LOG_DEBUG(
        "msg:Executing MPI_Alltoall.\t"
        "src:{:x}\tdest:{:x}\tnumber_of_bytes:{}"
    ,   reinterpret_cast<mgbase::intptr_t>(src)
    ,   reinterpret_cast<mgbase::intptr_t>(dest)
    ,   number_of_bytes
    );

    mpi_error::check(
        MPI_Alltoall(
            const_cast<void*>(src) // TODO: Only old versions of OpenMPI require
        ,   static_cast<int>(number_of_bytes)
        ,   MPI_BYTE
        ,   dest
        ,   static_cast<int>(number_of_bytes)
        ,   MPI_BYTE
        ,   MPI_COMM_WORLD // TODO
        )
    );
}

} // namespace untyped

} // namespace mpi
} // namespace mgcom

