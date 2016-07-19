
#pragma once

#include "device/mpi/mpi_interface.hpp"
#include "common/command/delegator.hpp"
#include "mpi_completer_base.hpp"

namespace mgcom {
namespace mpi {

class mpi_delegator
    : public virtual mpi_interface
{
public:
    mpi_delegator(endpoint& ep, delegator& del, mpi_completer_base& comp)
        : mpi_interface(ep)
        , del_(del)
        , comp_(comp)
        { }
    
    mpi_delegator(const mpi_delegator&) = delete;
    mpi_delegator& operator = (const mpi_delegator&) = delete;
    
    virtual bool try_irecv(const irecv_params& params) MGBASE_OVERRIDE;
    
    virtual bool try_isend(const isend_params& params) MGBASE_OVERRIDE;
    
    virtual bool try_irsend(const isend_params& params) MGBASE_OVERRIDE;
    
    virtual MPI_Comm comm_dup(MPI_Comm comm) MGBASE_OVERRIDE;
    
    virtual void comm_free(MPI_Comm* comm) MGBASE_OVERRIDE;
    
    virtual void comm_set_name(MPI_Comm comm, const char* comm_name) MGBASE_OVERRIDE;
    
    virtual bool try_native_barrier_async(const ibarrier_params& params) MGBASE_OVERRIDE;
    
    virtual bool try_native_broadcast_async(const ibcast_params& params) MGBASE_OVERRIDE;
    
    virtual bool try_native_allgather_async(const iallgather_params& params) MGBASE_OVERRIDE;
    
    virtual bool try_native_alltoall_async(const ialltoall_params& params) MGBASE_OVERRIDE;
    
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

