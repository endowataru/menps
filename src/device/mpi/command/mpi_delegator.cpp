
#include "mpi_delegator.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_error.hpp"
#include <mgbase/logger.hpp>
#include "common/command/delegate.hpp"

namespace mgcom {
namespace mpi {

// point-to-point communication

namespace /*unnamed*/ {

struct irecv_closure
{
    irecv_params        params;
    mpi_completer_base* comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Irecv(
                params.buf              // buf
            ,   params.size_in_bytes    // count
            ,   MPI_BYTE                // datatype
            ,   params.source_rank      // source
            ,   params.tag              // tag
            ,   params.comm             // comm
            ,   &request                // request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Irecv.\t"
            "buf:{:x}\tsize_in_bytes:{}\tsrc_rank:{}\ttag:{}\tcomm:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(params.buf)
        ,   params.size_in_bytes
        ,   params.source_rank
        ,   params.tag
        ,   get_comm_name(params.comm)
        );
        
        comp->complete({
            request
        ,   params.status_result
        ,   params.on_complete
        });
        
        return true;
    }
};

struct isend_closure
{
    isend_params        params;
    mpi_completer_base* comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Isend(
                const_cast<void*>(params.buf)   // buf
                //  Note: const_cast is required for old MPI libraries
            ,   params.size_in_bytes            // count
            ,   MPI_BYTE                        // datatype
            ,   params.dest_rank                // dest
            ,   params.tag                      // tag
            ,   params.comm                     // comm
            ,   &request                        // request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Isend.\t"
            "buf:{:x}\tsize_in_bytes:{}\tdest_rank:{}\ttag:{}\tcomm:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(params.buf)
        ,   params.size_in_bytes
        ,   params.dest_rank
        ,   params.tag
        ,   get_comm_name(params.comm)
        );
        
        comp->complete({
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct irsend_closure
{
    isend_params        params;
    mpi_completer_base* comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Irsend(
                const_cast<void*>(params.buf)   // buf
                // Note: const_cast is required for old MPI libraries
            ,   params.size_in_bytes            // count
            ,   MPI_BYTE                        // datatype
            ,   params.dest_rank                // dest
            ,   params.tag                      // tag
            ,   params.comm                     // comm
            ,   &request                        // request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Irsend.\t"
            "buf:{:x}\tsize_in_bytes:{}\tdest_rank:{}\ttag:{}\tcomm:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(params.buf)
        ,   params.size_in_bytes
        ,   params.dest_rank
        ,   params.tag
        ,   get_comm_name(params.comm)
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

bool mpi_delegator::try_irecv(const irecv_params& params)
{
    return try_delegate(
        del_
    ,   irecv_closure{ params, &comp_ }
    );
}

bool mpi_delegator::try_isend(const isend_params& params)
{
    return try_delegate(
        del_
    ,   isend_closure{ params, &comp_ }
    );
}

bool mpi_delegator::try_irsend(const isend_params& params)
{
    return try_delegate(
        del_
    ,   irsend_closure{ params, &comp_ }
    );
}


// collective communication

namespace /*unnamed*/ {

struct barrier_closure
{
    ibarrier_params p;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        MGBASE_LOG_DEBUG("msg:Executing MPI_Barrier.");
        
        const auto& params = p.base;
        
        mpi_error::check(
            MPI_Barrier(params.comm)
        );
        
        mgbase::execute(p.on_complete);
        
        return true;
    }
};

struct broadcast_closure
{
    ibcast_params p;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        const auto& params = p.base.coll;
        
        MGBASE_LOG_DEBUG(
            "msg:Executing MPI_Bcast.\t"
            "root:{}\tptr:{:x}\tnumber_of_bytes:{}"
        ,   params.root
        ,   reinterpret_cast<mgbase::intptr_t>(params.ptr)
        ,   params.num_bytes
        );
        
        mpi_error::check(
            MPI_Bcast(
                params.ptr // TODO
            ,   static_cast<int>(params.num_bytes)
            ,   MPI_BYTE
            ,   static_cast<int>(params.root)
            ,   p.base.comm
            )
        );
        
        mgbase::execute(p.on_complete);
        
        return true;
    }
};

struct allgather_closure
{
    iallgather_params p;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        const auto& params = p.base.coll;
        
        MGBASE_LOG_DEBUG(
            "msg:Executing MPI_Allgather.\t"
            "src:{:x}\tdest:{:x}\tnumber_of_bytes:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(params.src)
        ,   reinterpret_cast<mgbase::intptr_t>(params.dest)
        ,   params.num_bytes
        );
        
        mpi_error::check(
            MPI_Allgather(
                const_cast<void*>(params.src) // TODO: Only old versions of OpenMPI require
            ,   static_cast<int>(params.num_bytes)
            ,   MPI_BYTE
            ,   params.dest
            ,   static_cast<int>(params.num_bytes)
            ,   MPI_BYTE
            ,   p.base.comm
            )
        );
        
        mgbase::execute(p.on_complete);
        
        return true;
    }
};

struct alltoall_closure
{
    ialltoall_params p;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        const auto& params = p.base.coll;
        
        MGBASE_LOG_DEBUG(
            "msg:Executing MPI_Alltoall.\t"
            "src:{:x}\tdest:{:x}\tnumber_of_bytes:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(params.src)
        ,   reinterpret_cast<mgbase::intptr_t>(params.dest)
        ,   params.num_bytes
        );
        
        mpi_error::check(
            MPI_Alltoall(
                const_cast<void*>(params.src) // TODO: Only old versions of OpenMPI require
            ,   static_cast<int>(params.num_bytes)
            ,   MPI_BYTE
            ,   params.dest
            ,   static_cast<int>(params.num_bytes)
            ,   MPI_BYTE
            ,   p.base.comm
            )
        );
        
        mgbase::execute(p.on_complete);
        
        return true;
    }
};

} // unnamed namespace

bool mpi_delegator::try_native_barrier_async(const ibarrier_params& params)
{
    return try_delegate(
        del_
    ,   barrier_closure{ params }
    );
}

bool mpi_delegator::try_native_broadcast_async(const ibcast_params& params)
{
    return try_delegate(
        del_
    ,   broadcast_closure{ params }
    );
}

bool mpi_delegator::try_native_allgather_async(const iallgather_params& params)
{
    return try_delegate(
        del_
    ,   allgather_closure{ params }
    );
}

bool mpi_delegator::try_native_alltoall_async(const ialltoall_params& params)
{
    return try_delegate(
        del_
    ,   alltoall_closure{ params }
    );
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
        
        MGBASE_LOG_DEBUG(
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
        MGBASE_LOG_DEBUG(
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
        MGBASE_LOG_DEBUG(
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
    MGBASE_ASSERT(comm != MGBASE_NULLPTR);
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
} // namespace mgcom

