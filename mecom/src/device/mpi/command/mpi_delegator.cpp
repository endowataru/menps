
#include "mpi_delegator.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_error.hpp"
#include <menps/mefdn/logger.hpp>
#include "common/command/delegate.hpp"

namespace menps {
namespace mecom {
namespace mpi {

// point-to-point communication

namespace /*unnamed*/ {

struct recv_async_closure
{
    recv_async_params   params;
    mpi_completer_base* comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Irecv(
                pb.buf          // buf
            ,   pb.num_bytes    // count
            ,   MPI_BYTE        // datatype
            ,   pb.src_rank     // source
            ,   pb.tag          // tag
            ,   pb.comm         // comm
            ,   &request        // request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Irecv.\t"
            "buf:{:x}\tnum_bytes:{}\tsrc_rank:{}\ttag:{}\tcomm:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.buf)
        ,   pb.num_bytes
        ,   pb.src_rank
        ,   pb.tag
        ,   get_comm_name(pb.comm)
        );
        
        comp->complete({
            request
        ,   pb.status_result
        ,   params.on_complete
        });
        
        return true;
    }
};

struct send_async_closure
{
    send_async_params   params;
    mpi_completer_base* comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Isend(
                const_cast<void*>(pb.buf)   // buf
                //  Note: const_cast is required for old MPI libraries
            ,   pb.num_bytes                // count
            ,   MPI_BYTE                    // datatype
            ,   pb.dest_rank                // dest
            ,   pb.tag                      // tag
            ,   pb.comm                     // comm
            ,   &request                    // request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Isend.\t"
            "buf:{:x}\tnum_bytes:{}\tdest_rank:{}\ttag:{}\tcomm:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.buf)
        ,   pb.num_bytes
        ,   pb.dest_rank
        ,   pb.tag
        ,   get_comm_name(pb.comm)
        );
        
        comp->complete({
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

} // namespace unnamed

ult::async_status<void> mpi_delegator::recv_async(const recv_async_params params)
{
    delegate(
        del_
    ,   recv_async_closure{ params, &comp_ }
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi_delegator::send_async(const send_async_params params)
{
    delegate(
        del_
    ,   send_async_closure{ params, &comp_ }
    );
    
    return ult::make_async_deferred<void>();
}


// collective communication

namespace /*unnamed*/ {

struct barrier_closure
{
    barrier_async_params params;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        MEFDN_LOG_DEBUG("msg:Executing MPI_Barrier.");
        
        const auto& pb = params.base;
        
        mpi_error::check(
            MPI_Barrier(pb.comm)
        );
        
        // Execute the callback.
        params.on_complete();
        
        return true;
    }
};

struct broadcast_closure
{
    broadcast_async_params params;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        const auto& pb = params.base;
        
        MEFDN_LOG_DEBUG(
            "msg:Executing MPI_Bcast.\t"
            "root:{}\tptr:{:x}\tnumber_of_bytes:{}"
        ,   pb.root
        ,   reinterpret_cast<mefdn::intptr_t>(pb.ptr)
        ,   pb.num_bytes
        );
        
        mpi_error::check(
            MPI_Bcast(
                pb.ptr // TODO
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   static_cast<int>(pb.root)
            ,   pb.comm
            )
        );
        
        // Execute the callback.
        params.on_complete();
        
        return true;
    }
};

struct allgather_closure
{
    allgather_async_params params;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        const auto& pb = params.base;
        
        MEFDN_LOG_DEBUG(
            "msg:Executing MPI_Allgather.\t"
            "src:{:x}\tdest:{:x}\tnumber_of_bytes:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.src)
        ,   reinterpret_cast<mefdn::intptr_t>(pb.dest)
        ,   pb.num_bytes
        );
        
        mpi_error::check(
            MPI_Allgather(
                const_cast<void*>(pb.src) // TODO: Only old versions of OpenMPI require
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.dest
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.comm
            )
        );
        
        // Execute the callback.
        params.on_complete();
        
        return true;
    }
};

struct alltoall_closure
{
    alltoall_async_params params;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        const auto& pb = params.base;
        
        MEFDN_LOG_DEBUG(
            "msg:Executing MPI_Alltoall.\t"
            "src:{:x}\tdest:{:x}\tnumber_of_bytes:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.src)
        ,   reinterpret_cast<mefdn::intptr_t>(pb.dest)
        ,   pb.num_bytes
        );
        
        mpi_error::check(
            MPI_Alltoall(
                const_cast<void*>(pb.src) // TODO: Only old versions of OpenMPI require
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.dest
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.comm
            )
        );
        
        // Execute the callback.
        params.on_complete();
        
        return true;
    }
};

} // unnamed namespace

ult::async_status<void> mpi_delegator::barrier_async(const barrier_async_params params)
{
    delegate(
        del_
    ,   barrier_closure{ params }
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi_delegator::broadcast_async(const broadcast_async_params params)
{
    delegate(
        del_
    ,   broadcast_closure{ params }
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi_delegator::allgather_async(const allgather_async_params params)
{
    delegate(
        del_
    ,   allgather_closure{ params }
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi_delegator::alltoall_async(const alltoall_async_params params)
{
    delegate(
        del_
    ,   alltoall_closure{ params }
    );
    
    return ult::make_async_deferred<void>();
}


// communicator

namespace /*unnamed*/ {

struct comm_dup_closure
{
    MPI_Comm    old_comm;
    MPI_Comm*   new_comm;
    
    bool operator() () const
    {
        MPI_Comm result;
        
        MEFDN_LOG_DEBUG(
            "msg:Executing MPI_Comm_dup.\tcomm:{}"
        ,   get_comm_name(old_comm)
        );
        
        mpi_error::check(
            MPI_Comm_dup(old_comm, &result)
        );
        
        *new_comm = result;
        
        return true;
    }
};

struct comm_set_name_closure
{
    MPI_Comm    comm;
    const char* name;
    
    bool operator () () const
    {
        MEFDN_LOG_DEBUG(
            "msg:Executing MPI_Comm_set_name.\tcomm:{}\tname:{}"
        ,   get_comm_name(comm)
        ,   name
        );
        
        mpi_error::check(
            MPI_Comm_set_name(
                comm
            ,   const_cast<char*>(name) // const_cast is required for old MPI libraries
            )
        );
        
        return true;
    }
};

struct comm_free_closure
{
    MPI_Comm* comm;
    
    bool operator() () const
    {
        MEFDN_LOG_DEBUG(
            "msg:Executing MPI_Comm_free.\tcomm:{}"
        ,   get_comm_name(*comm)
        );
        
        mpi_error::check(
            MPI_Comm_free(comm)
        );
        
        return true;
    }
};


} // namespace unnamed

MPI_Comm mpi_delegator::comm_dup(const MPI_Comm comm)
{
    MPI_Comm result;
    
    execute(
        del_
    ,   comm_dup_closure{ comm, &result }
    );
    
    return result;
}

void mpi_delegator::comm_free(MPI_Comm* const comm)
{
    MEFDN_ASSERT(comm != nullptr);
    execute(del_, comm_free_closure{ comm });
}

void mpi_delegator::comm_set_name(const MPI_Comm comm, const char* const comm_name)
{
    execute(
        del_
    ,   comm_set_name_closure{ comm, comm_name }
    );
}

} // namespace mpi
} // namespace mecom
} // namespace menps

