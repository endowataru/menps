
#pragma once

#include "common/command/basic_command.hpp"
#include "device/mpi/mpi_base.hpp"
#include "mpi_completer.hpp"
#include <mgbase/operation.hpp>
#include <mgbase/logger.hpp>
#include <string> // for debugging

namespace mgcom {
namespace mpi {

union mpi_command_parameters
{
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

namespace /*unnamed*/ {

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_irecv(
    const mpi_command_parameters::irecv_parameters& params
,   mpi_completer&                                  completer
) {
    if (completer.full())
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
    ,   detail::get_comm_name(params.comm)
    );
    
    completer.complete(
        request
    ,   params.status_result
    ,   params.on_complete
    );
    
    return true;
}
 
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_isend(
    const mpi_command_parameters::isend_parameters& params
,   mpi_completer&                                  completer
) {
    if (completer.full())
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
    ,   detail::get_comm_name(params.comm)
    );
    
    completer.complete(
        request
    ,   MPI_STATUS_IGNORE
    ,   params.on_complete
    );
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_irsend(
    const mpi_command_parameters::isend_parameters& params
,   mpi_completer&                                  completer
) {
    if (completer.full())
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
    ,   detail::get_comm_name(params.comm)
    );
    
    completer.complete(
        request
    ,   MPI_STATUS_IGNORE
    ,   params.on_complete
    );
    
    return true;
}

} // unnamed namespace

#define MGCOM_MPI_COMMAND_CODES(x)  \
        x(MPI_COMMAND_IRECV)        \
    ,   x(MPI_COMMAND_ISEND)        \
    ,   x(MPI_COMMAND_IRSEND)

#define MGCOM_MPI_COMMAND_EXECUTE_CASES(CASE, RETURN, params, completer) \
    CASE(MPI_COMMAND_IRECV): { \
        const bool ret = ::mgcom::mpi::try_execute_irecv((params).irecv, (completer)); \
        RETURN(ret) \
    } \
    CASE(MPI_COMMAND_ISEND): { \
        const bool ret = ::mgcom::mpi::try_execute_isend((params).isend, (completer)); \
        RETURN(ret) \
    } \
    CASE(MPI_COMMAND_IRSEND): { \
        const bool ret = ::mgcom::mpi::try_execute_irsend((params).isend, (completer)); \
        RETURN(ret) \
    }

} // namespace mpi
} // namespace mgcom

