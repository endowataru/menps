
#pragma once

#include "common/command/basic_command.hpp"
#include "device/mpi/mpi_base.hpp"
#include "mpi_completer.hpp"
#include "common/command/comm_lock.hpp"
#include <mgbase/threading/synchronizer.hpp>
#include <mgbase/operation.hpp>
#include <mgbase/logger.hpp>
#include <string> // for debugging

namespace mgcom {
namespace mpi {

enum mpi_command_code
{
    MPI_COMMAND_IRECV = BASIC_COMMAND_END
,   MPI_COMMAND_ISEND
,   MPI_COMMAND_IRSEND
,   MPI_COMMAND_END
};

union mpi_command_parameters
{
#if 0
    struct lock_parameters
    {
        comm_lock*                  lock;
    }
    lock;
#endif
    basic_command_parameters        basic;
    
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
    if (code < static_cast<mpi_command_code>(BASIC_COMMAND_END)) {
    }
    
    MGBASE_ASSERT(MPI_COMMAND_IRECV <= code && code < MPI_COMMAND_END);
    
    switch (code)
    {
        #if 0
        case MPI_COMMAND_LOCK: {
            const mpi_command_parameters::lock_parameters& p = params.lock;
            
            MGBASE_LOG_DEBUG("msg:Communication thread was locked by other thread.");
            
            p.lock->wait();
            
            /*{
                mgbase::unique_lock<mgbase::mutex> lc(*p.mtx);
                p.sync->wait(lc);
            }*/
            
            MGBASE_LOG_DEBUG("msg:Communication thread was unlocked.");
            
            return true;
        }
        #endif
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
        
        case MPI_COMMAND_END:
        default: {
            return execute_on_this_thread(
                static_cast<basic_command_code>(code)
            ,   params.basic
            );
        }
    }
}

} // namespace mpi
} // namespace mgcom

