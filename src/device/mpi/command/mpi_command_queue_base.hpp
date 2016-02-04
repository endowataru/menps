
#pragma once

#include "mpi_command.hpp"

namespace mgcom {
namespace mpi {

class mpi_command_queue_base
{
protected:
    mpi_command_queue_base() MGBASE_EMPTY_DEFINITION
    
    virtual ~mpi_command_queue_base() MGBASE_EMPTY_DEFINITION
    
    virtual bool try_enqueue_mpi(
        const mpi_command_code          code
    ,   const mpi_command_parameters&   params
    ) = 0;
    
public:
    bool try_lock()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_, mgbase::try_to_lock);
        
        if (lc.owns_lock())
            return try_enqueue_mpi_lock(lc);
        else
            return false;
    }
    
    void lock()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        
        while (!try_enqueue_mpi_lock(lc)) { }
    }
    
    void unlock()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        
        sync_.finish(lc);
    }
    
private:
    bool try_enqueue_mpi_lock(mgbase::unique_lock<mgbase::mutex>& lc)
    {
        MGBASE_ASSERT(lc.owns_lock());
        
        const mpi_command_parameters::lock_parameters params = {
            &this->mtx_
        ,   &this->sync_
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
        
        if (ret)
        {
            sync_.start(lc);
            return true;
        }
        else
            return false;
    }
    
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
    
    #if 0
    bool try_test(
        MPI_Request* const          request
    ,   bool* const                 success_result
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi_command_parameters::test_parameters params = {
            request
        ,   success_result
        ,   on_complete
        };
        
        mpi_command_parameters mpi_params;
        mpi_params.test = params;
        
        const bool ret = try_enqueue_mpi(
            MPI_COMMAND_TEST
        ,   mpi_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\trequest:{:x}"
        ,   (ret ? "Queued MPI_Test." : "Failed to queue MPI_Test.")
        ,   reinterpret_cast<mgbase::uintptr_t>(request)
        );
        
        return ret;
    }
    
    bool try_testany(
        const int                   count
    ,   MPI_Request* const          first_request
    ,   int* const                  index_result
    ,   bool* const                 success_result
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi_command_parameters::testany_parameters params = {
            count
        ,   first_request
        ,   index_result
        ,   success_result
        ,   on_complete
        };
        
        mpi_command_parameters mpi_params;
        mpi_params.testany = params;
        
        const bool ret = try_enqueue_mpi(
            MPI_COMMAND_TESTANY
        ,   mpi_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:\tcount:{}\tfirst_request:{:x}"
        ,   (ret ? "Queued MPI_Testany." : "Failed to queue MPI_Testany.")
        ,   count
        ,   reinterpret_cast<mgbase::uintptr_t>(first_request)
        );
        
        return ret;
    }
    #endif
    
private:
    mgbase::mutex           mtx_;
    mgbase::synchronizer    sync_;
};

} // namespace mpi
} // namespace mgcom

