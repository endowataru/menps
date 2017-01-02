
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
    
    virtual void get_async(get_async_params) MGBASE_OVERRIDE;
    
    virtual void put_async(put_async_params) MGBASE_OVERRIDE;
    
    virtual void compare_and_swap_async(compare_and_swap_async_params) MGBASE_OVERRIDE;
    
    virtual void fetch_and_op_async(fetch_and_op_async_params) MGBASE_OVERRIDE;
    
    virtual MPI_Aint attach(attach_params) MGBASE_OVERRIDE;
    
    virtual void detach(detach_params) MGBASE_OVERRIDE;
    
    // mpi1_interface
    
    virtual void barrier_async(barrier_async_params) MGBASE_OVERRIDE;
    
    virtual void broadcast_async(broadcast_async_params) MGBASE_OVERRIDE;
    
    virtual void allgather_async(allgather_async_params) MGBASE_OVERRIDE;
    
    virtual void alltoall_async(alltoall_async_params) MGBASE_OVERRIDE;
};

} // namespace mpi3
} // namespace mgcom

