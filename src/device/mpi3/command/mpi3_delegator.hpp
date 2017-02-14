
#pragma once

#include "device/mpi/command/mpi_delegator.hpp"
#include "device/mpi3/mpi3_interface.hpp"

namespace mgcom {
namespace mpi3 {

using mgdev::mpi::get_async_params;
using mgdev::mpi::put_async_params;
using mgdev::mpi::compare_and_swap_async_params;
using mgdev::mpi::fetch_and_op_async_params;
using mgdev::mpi::attach_params;
using mgdev::mpi::detach_params;

using mgdev::mpi::barrier_async_params;
using mgdev::mpi::broadcast_async_params;
using mgdev::mpi::allgather_async_params;
using mgdev::mpi::alltoall_async_params;

class mpi3_delegator
    : public mpi::mpi_delegator
    , public virtual mpi3_interface
{
public:
    mpi3_delegator(delegator& del, mpi::mpi_completer_base& comp)
        : mpi::mpi_delegator(del, comp)
    { }
    
    // mpi3_interface
    
    virtual ult::async_status<void> get_async(get_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual ult::async_status<void> put_async(put_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual ult::async_status<void> compare_and_swap_async(compare_and_swap_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual ult::async_status<void> fetch_and_op_async(fetch_and_op_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual MPI_Aint attach(attach_params) MGBASE_OVERRIDE;
    
    virtual void detach(detach_params) MGBASE_OVERRIDE;
    
    // mpi1_interface
    
    virtual ult::async_status<void> barrier_async(barrier_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual ult::async_status<void> broadcast_async(broadcast_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual ult::async_status<void> allgather_async(allgather_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
    
    virtual ult::async_status<void> alltoall_async(alltoall_async_params)
        MGBASE_OVERRIDE MGBASE_WARN_UNUSED_RESULT;
};

} // namespace mpi3
} // namespace mgcom

