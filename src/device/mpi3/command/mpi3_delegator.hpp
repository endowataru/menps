
#pragma once

#include "device/mpi/command/mpi_delegator.hpp"
#include "device/mpi3/mpi3_interface.hpp"

namespace mgcom {
namespace mpi3 {

class mpi3_delegator
    : public mpi::mpi_delegator
    , public virtual mpi3_interface
{
public:
    mpi3_delegator(endpoint& ep, delegator& del, mpi::mpi_completer_base& comp, MPI_Win win)
        : mpi_interface(ep)
        , mpi3_interface(ep)
        , mpi::mpi_delegator(ep, del, comp)
        , win_(win) { }
    
    // mpi3_interface
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_rget(const rget_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_rput(const rput_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_compare_and_swap(const compare_and_swap_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_fetch_and_op(const fetch_and_op_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_ibarrier(const ibarrier_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_ibcast(const ibcast_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_iallgather(const iallgather_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_ialltoall(const ialltoall_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_attach_async(const attach_async_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_detach_async(const detach_async_params&) MGBASE_OVERRIDE;
    
    // mpi_interface
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_barrier_async(const ibarrier_params& params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_broadcast_async(const ibcast_params& params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_allgather_async(const iallgather_params& params) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_alltoall_async(const ialltoall_params& params) MGBASE_OVERRIDE;
    
private:
    MPI_Win win_;
};

} // namespace mpi3
} // namespace mgcom

