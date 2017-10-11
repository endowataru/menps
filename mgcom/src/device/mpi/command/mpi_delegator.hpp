
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include "common/command/delegator.hpp"
#include "mpi_completer_base.hpp"
#include <mgcom/ult.hpp>

namespace mgcom {
namespace mpi {

using mgdev::mpi::recv_async_params;
using mgdev::mpi::send_async_params;

using mgdev::mpi::barrier_async_params;
using mgdev::mpi::broadcast_async_params;
using mgdev::mpi::allgather_async_params;
using mgdev::mpi::alltoall_async_params;

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
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> recv_async(recv_async_params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> send_async(send_async_params) MGBASE_OVERRIDE;
    
    // Collective communication
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> barrier_async(barrier_async_params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> broadcast_async(broadcast_async_params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> allgather_async(allgather_async_params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> alltoall_async(alltoall_async_params) MGBASE_OVERRIDE;
    
    // Communicators
    
    virtual MPI_Comm comm_dup(MPI_Comm) MGBASE_OVERRIDE;
    
    virtual void comm_free(MPI_Comm*) MGBASE_OVERRIDE;
    
    virtual void comm_set_name(MPI_Comm, const char*) MGBASE_OVERRIDE;
    
protected:
    delegator& get_delegator() const MGBASE_NOEXCEPT {
        return del_;
    }
    
    mpi_completer_base& get_completer() const MGBASE_NOEXCEPT {
        return comp_;
    }
    
private:
    delegator&          del_;
    mpi_completer_base& comp_;
};

} // namespace mpi
} // namespace mgcom

