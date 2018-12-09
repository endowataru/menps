
#pragma once

#include "device/mpi/command/mpi_delegator.hpp"
#include "device/mpi3/mpi3_interface.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

using medev::mpi::get_async_params;
using medev::mpi::put_async_params;
using medev::mpi::compare_and_swap_async_params;
using medev::mpi::fetch_and_op_async_params;
using medev::mpi::attach_params;
using medev::mpi::detach_params;

using medev::mpi::barrier_async_params;
using medev::mpi::broadcast_async_params;
using medev::mpi::allgather_async_params;
using medev::mpi::alltoall_async_params;

class mpi3_delegator
    : public mpi::mpi_delegator
    , public virtual mpi3_interface
{
public:
    mpi3_delegator(delegator& del, mpi::mpi_completer_base& comp)
        : mpi::mpi_delegator(del, comp)
    { }
    
    // mpi3_interface
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> get_async(get_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> put_async(put_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> compare_and_swap_async(compare_and_swap_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> fetch_and_op_async(fetch_and_op_async_params) MEFDN_OVERRIDE;
    
    virtual MPI_Aint attach(attach_params) MEFDN_OVERRIDE;
    
    virtual void detach(detach_params) MEFDN_OVERRIDE;
    
    // mpi1_interface
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> barrier_async(barrier_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> broadcast_async(broadcast_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> allgather_async(allgather_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> alltoall_async(alltoall_async_params) MEFDN_OVERRIDE;
};

} // namespace mpi3
} // namespace mecom
} // namespace menps

