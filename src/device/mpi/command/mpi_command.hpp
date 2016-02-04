
#pragma once

#include "device/mpi/mpi_base.hpp"
#include "mpi_completer.hpp"
#include <mgbase/threading/synchronizer.hpp>
#include <mgbase/operation.hpp>
#include <mgbase/logger.hpp>
#include <string> // for debugging

namespace mgcom {
namespace mpi {

enum mpi_command_code
{
    MPI_COMMAND_LOCK
,   MPI_COMMAND_IRECV
,   MPI_COMMAND_ISEND
,   MPI_COMMAND_IRSEND
#if 0
,   MPI_COMMAND_TEST
,   MPI_COMMAND_TESTANY
#endif
,   MPI_COMMAND_END
};

union mpi_command_parameters
{
    struct lock_parameters
    {
        mgbase::mutex*              mtx;
        mgbase::synchronizer*       sync;
    }
    lock;
    
    struct irecv_parameters
    {
        void*                       buf;
        int                         size_in_bytes;
        int                         source_rank;
        int                         tag;
        MPI_Comm                    comm;
        MPI_Status*                 status_result;
        mgbase::operation           on_complete;
    }
    irecv;
    
    struct isend_parameters
    {
        const void*                 buf;
        int                         size_in_bytes;
        int                         dest_rank;
        int                         tag;
        MPI_Comm                    comm;
        mgbase::operation           on_complete;
    }
    isend;
    
    #if 0
    struct test_parameters
    {
        MPI_Request*                request;
        bool*                       success_result;
        mgbase::operation           on_complete;
    }
    test;
    
    struct testany_parameters
    {
        int                         count;
        MPI_Request*                first_request;
        int*                        index_result;
        bool*                       success_result;
        mgbase::operation           on_complete;
    }
    testany;
    #endif
};

namespace detail {

inline std::string get_comm_name(const MPI_Comm comm)
{
    char name[MPI_MAX_OBJECT_NAME];
    int len;
    
    mpi_error::check(
        MPI_Comm_get_name(comm, name, &len)
    );
    
    return name;
}

} // namespace detail

MGBASE_ALWAYS_INLINE bool execute_on_this_thread(
    const mpi_command_code          code
,   const mpi_command_parameters&   params
,   mpi_completer&                  completer
) {
    switch (code)
    {
        case MPI_COMMAND_LOCK: {
            const mpi_command_parameters::lock_parameters& p = params.lock;
            
            MGBASE_LOG_DEBUG("msg:Communication thread was locked by other thread.");
            
            {
                mgbase::unique_lock<mgbase::mutex> lc(*p.mtx);
                p.sync->wait(lc);
            }
            
            MGBASE_LOG_DEBUG("msg:Communication thread was unlocked.");
            
            return true;
        }
        
        case MPI_COMMAND_IRECV: {
            const mpi_command_parameters::irecv_parameters& p = params.irecv;
            
            if (completer.full())
                return false;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Irecv(
                    p.buf               // buf
                ,   p.size_in_bytes     // count
                ,   MPI_BYTE            // datatype
                ,   p.source_rank       // source
                ,   p.tag               // tag
                ,   p.comm              // comm
                ,   &request            // request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Irecv.\t"
                "buf:{:x}\tsize_in_bytes:{}\tsrc_rank:{}\ttag:{}\tcomm:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.buf)
            ,   p.size_in_bytes
            ,   p.source_rank
            ,   p.tag
            ,   detail::get_comm_name(p.comm)
            );
            
            completer.complete(
                request
            ,   p.status_result
            ,   p.on_complete
            );
            
            return true;
        }
        
        case MPI_COMMAND_ISEND: {
            const mpi_command_parameters::isend_parameters& p = params.isend;
            
            if (completer.full())
                return false;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Isend(
                    const_cast<void*>(p.buf)    // buf (Note: const_cast is required for old MPI libraries)
                ,   p.size_in_bytes             // count
                ,   MPI_BYTE                    // datatype
                ,   p.dest_rank                 // dest
                ,   p.tag                       // tag
                ,   p.comm                      // comm
                ,   &request                    // request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Isend.\t"
                "buf:{:x}\tsize_in_bytes:{}\tdest_rank:{}\ttag:{}\tcomm:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.buf)
            ,   p.size_in_bytes
            ,   p.dest_rank
            ,   p.tag
            ,   detail::get_comm_name(p.comm)
            );
            
            completer.complete(
                request
            ,   MPI_STATUS_IGNORE
            ,   p.on_complete
            );
            
            return true;
        }
        
        case MPI_COMMAND_IRSEND: {
            const mpi_command_parameters::isend_parameters& p = params.isend;
            
            if (completer.full())
                return false;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Irsend(
                    const_cast<void*>(p.buf)    // buf (Note: const_cast is required for old MPI libraries)
                ,   p.size_in_bytes             // count
                ,   MPI_BYTE                    // datatype
                ,   p.dest_rank                 // dest
                ,   p.tag                       // tag
                ,   p.comm                      // comm
                ,   &request                    // request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Irsend.\t"
                "buf:{:x}\tsize_in_bytes:{}\tdest_rank:{}\ttag:{}\tcomm:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.buf)
            ,   p.size_in_bytes
            ,   p.dest_rank
            ,   p.tag
            ,   detail::get_comm_name(p.comm)
            );
            
            completer.complete(
                request
            ,   MPI_STATUS_IGNORE
            ,   p.on_complete
            );
            
            return true;
        }
        
        #if 0
        case MPI_COMMAND_TEST: {
            const mpi_command_parameters::test_parameters& p = params.test;
            
            int flag;
            MPI_Status status;
            
            mpi_error::check(
                MPI_Test(
                    p.request   // request
                ,   &flag       // flag
                ,   &status     // status
                )
            );
            
            *p.success_result = (flag == 1);
            // status is ignored
            
            mgbase::execute(p.on_complete);
            return true;
        }
        
        case MPI_COMMAND_TESTANY: {
            const mpi_command_parameters::testany_parameters& p = params.testany;
            
            int flag;
            MPI_Status status;
            
            mpi_error::check(
                MPI_Testany(
                    p.count             // count
                ,   p.first_request     // array_of_requests
                ,   p.index_result      // idx
                ,   &flag               // flag
                ,   &status             // status
                )
            );
            
            *p.success_result = (flag == 1);
            // status is ignored
            
            mgbase::execute(p.on_complete);
            return true;
        }
        #endif
        
        default:
            MGBASE_UNREACHABLE();
            mpi_error::emit();
            break;
    }
}

} // namespace mpi
} // namespace mgcom

