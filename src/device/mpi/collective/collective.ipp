
#pragma once

#include <mgcom/collective.hpp>
#include <mgbase/logging/logger.hpp>

#include <mgbase/threading/lock_guard.hpp>

#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_error.hpp"

namespace mgcom {
namespace collective {

namespace detail {

class broadcast_handlers
{
    typedef broadcast_handlers      handlers_type;
    typedef broadcast_cb            cb_type;
    typedef mgbase::deferred<void>  result_type;
    
public:
    static result_type start(cb_type& cb)
    {
        return mgbase::add_continuation<result_type (cb_type&), &handlers_type::transfer>(
            cb
        ,   mgcom::collective::barrier_nb(cb.sync.cb_barrier)
        );
    }

private:
    static result_type transfer(cb_type& cb)
    {
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            MGBASE_LOG_DEBUG("msg:Blocking broadcast.\troot:{}\tptr:{:x}\tnumber_of_bytes:{}",
                cb.root, reinterpret_cast<mgbase::uint64_t>(cb.ptr), cb.number_of_bytes);
            
            mpi_error::check(
                MPI_Bcast(
                    cb.ptr // TODO
                ,   cb.number_of_bytes
                ,   MPI_BYTE
                ,   static_cast<int>(cb.root)
                ,   MPI_COMM_WORLD // TODO
                )
            );
        }
        
        return mgbase::make_ready_deferred();
    }
};

class allgather_handlers
{
    typedef allgather_handlers      handlers_type;
    typedef allgather_cb            cb_type;
    typedef mgbase::deferred<void>  result_type;
    
public:
    static result_type start(cb_type& cb)
    {
        return mgbase::add_continuation<result_type (cb_type&), &handlers_type::transfer>(
            cb
        ,   mgcom::collective::barrier_nb(cb.sync.cb_barrier)
        );
    }

private:
    static result_type transfer(cb_type& cb)
    {
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
            
            MGBASE_LOG_DEBUG("msg:Blocking allgather.\tsrc:{:x}\tdest:{:x}\tnumber_of_bytes:{}",
                reinterpret_cast<mgbase::uint64_t>(cb.src), reinterpret_cast<mgbase::uint64_t>(cb.dest), cb.number_of_bytes);
            
            mpi_error::check(
                MPI_Allgather(
                    const_cast<void*>(cb.src) // TODO: Only old versions of OpenMPI require
                ,   cb.number_of_bytes
                ,   MPI_BYTE
                ,   cb.dest
                ,   cb.number_of_bytes
                ,   MPI_BYTE
                ,   MPI_COMM_WORLD // TODO
                )
            );
        }
        
        return mgbase::make_ready_deferred();
    }
};

} // namespace detail

} // namespace collective
} // namespace mgcom

