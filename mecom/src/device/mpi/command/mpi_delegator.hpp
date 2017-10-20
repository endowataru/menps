
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include "common/command/delegator.hpp"
#include "mpi_completer_base.hpp"
#include <menps/mecom/ult.hpp>

namespace menps {
namespace mecom {
namespace mpi {

using medev::mpi::recv_async_params;
using medev::mpi::send_async_params;

using medev::mpi::barrier_async_params;
using medev::mpi::broadcast_async_params;
using medev::mpi::allgather_async_params;
using medev::mpi::alltoall_async_params;

class mpi_delegator
    : public virtual mpi_interface
{
public:
    mpi_delegator(delegator& del, mpi_completer_base& comp)
        : del_(del)
        , comp_(comp)
    { }
    
    mpi_delegator(const mpi_delegator&) = delete;
    mpi_delegator& operator = (const mpi_delegator&) = delete;
    
    // Point-to-point communication
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> recv_async(recv_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> send_async(send_async_params) MEFDN_OVERRIDE;
    
    // Collective communication
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> barrier_async(barrier_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> broadcast_async(broadcast_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> allgather_async(allgather_async_params) MEFDN_OVERRIDE;
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> alltoall_async(alltoall_async_params) MEFDN_OVERRIDE;
    
    // Communicators
    
    virtual MPI_Comm comm_dup(MPI_Comm) MEFDN_OVERRIDE;
    
    virtual void comm_free(MPI_Comm*) MEFDN_OVERRIDE;
    
    virtual void comm_set_name(MPI_Comm, const char*) MEFDN_OVERRIDE;
    
protected:
    delegator& get_delegator() const noexcept {
        return del_;
    }
    
    mpi_completer_base& get_completer() const noexcept {
        return comp_;
    }
    
private:
    delegator&          del_;
    mpi_completer_base& comp_;
};

} // namespace mpi
} // namespace mecom
} // namespace menps

