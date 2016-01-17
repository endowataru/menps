
#pragma once

#include <mgcom.hpp>
#include "device/mpi/mpi_base.hpp"
#include <mgbase/logging/logger.hpp>
#include <mgbase/threading/lock_guard.hpp>

namespace mgcom {
namespace collective {

namespace detail {

class barrier_handlers
{
    typedef barrier_cb  cb_type;
    
public:
    static mgbase::deferred<void> start(cb_type& cb)
    {
        MGBASE_LOG_DEBUG("msg:Started barrier.");
        
        // TODO: dynamic allocation for collective
        MPI_Request* request = new MPI_Request;
        cb.request = request;
        
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            mpi_error::check(
                MPI_Ibarrier(
                    MPI_COMM_WORLD // TODO
                ,   request
                )
            );
        }
        
        return test(cb);
    }

private:
    static mgbase::deferred<void> test(cb_type& cb)
    {
        MPI_Request* request = static_cast<MPI_Request*>(cb.request);
        
        int flag;
        MPI_Status status;
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            mpi_error::check(
                MPI_Test(request, &flag, &status)
            );
        }
        
        if (flag) 
        {
            delete request;
            
            MGBASE_LOG_DEBUG("msg:Finished barrier.");
            
            return mgbase::make_ready_deferred();
        }
        
        // TODO: Selective polling
        mgcom::am::poll();
        
        return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), test>(cb);
    }
};

class broadcast_handlers
{
    typedef broadcast_cb  cb_type;
    
public:
    static mgbase::deferred<void> start(cb_type& cb)
    {
        MGBASE_LOG_DEBUG("msg:Started broadcast.");
        
        // TODO: dynamic allocation for collective
        MPI_Request* request = new MPI_Request;
        cb.sync.request = request;
        
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            mpi_error::check(
                MPI_Ibcast(
                    cb.ptr // TODO
                ,   cb.number_of_bytes
                ,   MPI_BYTE
                ,   static_cast<int>(cb.root)
                ,   MPI_COMM_WORLD // TODO
                ,   request
                )
            );
        }
        
        return test(cb);
    }

private:
    static mgbase::deferred<void> test(cb_type& cb)
    {
        MPI_Request* request = static_cast<MPI_Request*>(cb.sync.request);
        
        int flag;
        MPI_Status status;
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            mpi_error::check(
                MPI_Test(request, &flag, &status)
            );
        }
        
        if (flag) 
        {
            delete request;
            
            MGBASE_LOG_DEBUG("msg:Finished broadcast.");
            
            return mgbase::make_ready_deferred();
        }
        
        // TODO: Selective polling
        mgcom::am::poll();
        
        return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), test>(cb);
    }
};

class allgather_handlers
{
    typedef allgather_cb    cb_type;
    
public:
    static mgbase::deferred<void> start(cb_type& cb)
    {
        MGBASE_LOG_DEBUG("msg:Started allgather.");
        
        // TODO: dynamic allocation for collective
        MPI_Request* request = new MPI_Request;
        cb.sync.request = request;
        
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            mpi_error::check(
                MPI_Iallgather(
                    cb.src
                ,   cb.number_of_bytes
                ,   MPI_BYTE
                ,   cb.dest
                ,   cb.number_of_bytes
                ,   MPI_BYTE
                ,   MPI_COMM_WORLD // TODO
                ,   request
                )
            );
        }
        
        return test(cb);
    }

private:
    static mgbase::deferred<void> test(cb_type& cb)
    {
        MPI_Request* request = static_cast<MPI_Request*>(cb.sync.request);
        
        int flag;
        MPI_Status status;
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            mpi_error::check(
                MPI_Test(request, &flag, &status)
            );
        }
        
        if (flag) 
        {
            delete request;
            
            MGBASE_LOG_DEBUG("msg:Finished allgather.");
            
            return mgbase::make_ready_deferred();
        }
        
        // TODO: Selective polling
        mgcom::am::poll();
        
        return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), test>(cb);
    }
};


} // namespace detail

} // namespace collective
} // namespace mgcom

