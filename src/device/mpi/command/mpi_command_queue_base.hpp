
#pragma once

#include "mpi_command.hpp"
#include "common/command/comm_lock.hpp"

namespace mgcom {
namespace mpi {

class mpi_command_queue_base
    : public comm_lock
{
protected:
    mpi_command_queue_base() { }
    
    virtual bool try_enqueue_mpi(
        const mpi_command_code          code
    ,   const mpi_command_parameters&   params
    ) = 0;
    
public:
    bool try_irecv(
        void* const                 buf
    ,   const int                   size_in_bytes
    ,   const int                   source_rank
    ,   const int                   tag
    ,   const MPI_Comm              comm
    ,   MPI_Status* const           status_result
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi_command_parameters::irecv_parameters params = {
            buf
        ,   size_in_bytes
        ,   source_rank
        ,   tag
        ,   comm
        ,   status_result
        ,   on_complete
        };
        
        mpi_command_parameters mpi_params;
        mpi_params.irecv = params;
        
        const bool ret = try_enqueue_mpi(
            MPI_COMMAND_IRECV
        ,   mpi_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\tbuf:{:x}\tsize_in_bytes:{}\tsource_rank:{}\ttag:{}"
        ,   (ret ? "Queued MPI_Irecv." : "Failed to queue MPI_Irecv.")
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   size_in_bytes
        ,   source_rank
        ,   tag
        );
        
        return ret;
    }
    
    bool try_isend(
        const void* const           buf
    ,   const int                   size_in_bytes
    ,   const int                   dest_rank
    ,   const int                   tag
    ,   const MPI_Comm              comm
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi_command_parameters::isend_parameters params = {
            buf
        ,   size_in_bytes
        ,   dest_rank
        ,   tag
        ,   comm
        ,   on_complete
        };
        
        mpi_command_parameters mpi_params;
        mpi_params.isend = params;
        
        const bool ret = try_enqueue_mpi(
            MPI_COMMAND_ISEND
        ,   mpi_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\tbuf:{:x}\tsize_in_bytes:{}\tdest_rank:{}\ttag:{}"
        ,   (ret ? "Queued MPI_Isend." : "Failed to queue MPI_Isend.")
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   size_in_bytes
        ,   dest_rank
        ,   tag
        );
        
        return ret;
    }
    
    bool try_irsend(
        const void* const           buf
    ,   const int                   size_in_bytes
    ,   const int                   dest_rank
    ,   const int                   tag
    ,   const MPI_Comm              comm
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi_command_parameters::isend_parameters params = {
            buf
        ,   size_in_bytes
        ,   dest_rank
        ,   tag
        ,   comm
        ,   on_complete
        };
        
        mpi_command_parameters mpi_params;
        mpi_params.isend = params;
        
        const bool ret = try_enqueue_mpi(
            MPI_COMMAND_IRSEND
        ,   mpi_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\tbuf:{:x}\tsize_in_bytes:{}\tdest_rank:{}\ttag:{}"
        ,   (ret ? "Queued MPI_Irsend." : "Failed to queue MPI_Irsend.")
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   size_in_bytes
        ,   dest_rank
        ,   tag
        );
        
        return ret;
    }

protected:
    virtual bool try_enqueue_unlock() MGBASE_OVERRIDE
    {
        const mpi_command_parameters::lock_parameters params = {
            this
        };
        
        mpi_command_parameters mpi_params;
        mpi_params.lock = params;
        
        const bool ret = try_enqueue_mpi(
            MPI_COMMAND_LOCK
        ,   mpi_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}"
        ,   (ret ? "Queued MPI lock." : "Failed to queue MPI lock.")
        );
        
        return ret;
    }
};

} // namespace mpi
} // namespace mgcom

