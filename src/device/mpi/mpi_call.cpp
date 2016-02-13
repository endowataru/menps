
#include "mpi_call.hpp"
#include "mpi_base.hpp"
#include "common/command/comm_call.hpp"

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

inline void execute_barrier()
{
    MGBASE_LOG_DEBUG("msg:Executing MPI_Barrier.");
    
    mpi_error::check(
        MPI_Barrier(
            MPI_COMM_WORLD // TODO
        )
    );
}

} // unnamed namespace

void native_barrier()
{
    comm_call<void>(
        MGBASE_MAKE_INLINED_FUNCTION(&execute_barrier)
    );
}

namespace untyped {

namespace /*unnamed*/ {

struct broadcast_closure
{
    void operator () () const
    {
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
    
    process_id_t    root;
    void*           ptr;
    index_t         number_of_bytes;
};

} // unnamed namespace

void native_broadcast(
    const process_id_t  root
,   void* const         ptr
,   const index_t       number_of_bytes
) {
    const broadcast_closure cl = { root, ptr, number_of_bytes };
    comm_call<void>(cl);
}

namespace /*unnamed*/ {

struct allgather_closure
{
    void operator () () const
    {
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
    
    const void*     src;
    void*           dest;
    index_t         number_of_bytes;
};

} // unnamed namespace

void native_allgather(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    const allgather_closure cl = { src, dest, number_of_bytes };
    comm_call<void>(cl);
}

namespace /*unnamed*/ {

struct alltoall_closure
{
    void operator () () const
    {
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
    
    const void*     src;
    void*           dest;
    index_t         number_of_bytes;
};

} // unnamed namespace

void native_alltoall(
    const void* const   src
,   void* const         dest
,   const index_t       number_of_bytes
) {
    const alltoall_closure cl = { src, dest, number_of_bytes };
    comm_call<void>(cl);
}

} // namespace untyped

namespace /*unnamed*/ {

struct comm_dup_arg
{
    MPI_Comm comm;
};

inline MPI_Comm execute_comm_dup(const comm_dup_arg& arg)
{
    MPI_Comm result;
    
    mpi_error::check(
        MPI_Comm_dup(arg.comm, &result)
    );
    
    return result;
}

} // namespace unnamed

MPI_Comm comm_dup(const MPI_Comm comm)
{
    const comm_dup_arg arg = { comm };
    
    const MPI_Comm result
        = comm_call<MPI_Comm>(
            mgbase::bind_ref1(
                MGBASE_MAKE_INLINED_FUNCTION(execute_comm_dup)
            ,   arg
            )
        );
    
    MGBASE_LOG_DEBUG("msg:Duplicated communicator.");
    
    return result;
}

namespace /*unnamed*/ {

struct comm_set_name_arg
{
    MPI_Comm    comm;
    const char* name;
};

inline void execute_comm_set_name(const comm_set_name_arg& arg)
{
    mpi_error::check(
        MPI_Comm_set_name(
            arg.comm
        ,   const_cast<char*>(arg.name) // const_cast is required for old MPI libraries
        )
    );
}

} // namespace unnamed

void comm_set_name(const MPI_Comm comm, const char* const comm_name)
{
    const comm_set_name_arg arg = { comm, comm_name };
    
    comm_call<void>(
        mgbase::bind_ref1(
            MGBASE_MAKE_INLINED_FUNCTION(execute_comm_set_name)
        ,   arg
        )
    );
    
    MGBASE_LOG_DEBUG("msg:Set communicator name.\tname:{}", comm_name);
}

} // namespace mpi
} // namespace mgcom

