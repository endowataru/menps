
#pragma once

#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi3 {

using mpi::ibarrier_params;
using mpi::ibcast_params;
using mpi::iallgather_params;
using mpi::ialltoall_params;

struct rget_params
{
    void*               dest_ptr;
    int                 src_rank;
    MPI_Aint            src_index;
    int                 size_in_bytes;
    mgbase::callback<void ()>   on_complete;
};

struct rput_params
{
    const void*         src_ptr;
    int                 dest_rank;
    MPI_Aint            dest_index;
    int                 size_in_bytes;
    mgbase::callback<void ()>   on_complete;
};

struct compare_and_swap_params
{
    mgbase::uint64_t    expected;
    mgbase::uint64_t    desired;
    void*               result_ptr;
    MPI_Datatype        datatype;
    int                 dest_rank;
    MPI_Aint            dest_index;
    mgbase::callback<void ()>   on_complete;
};

struct fetch_and_op_params
{
    mgbase::uint64_t    value;
    void*               result_ptr;
    MPI_Datatype        datatype;
    int                 dest_rank;
    MPI_Aint            dest_index;
    MPI_Op              operation;
    mgbase::callback<void ()>   on_complete;
};

struct attach_params
{
    void*       ptr;
    MPI_Aint    size;
};

struct attach_async_params
{
    attach_params       base;
    MPI_Aint*           addr_result;
    mgbase::callback<void ()>   on_complete;
};

struct detach_params
{
    void*   ptr;
};

struct detach_async_params
{
    detach_params       base;
    mgbase::callback<void ()>   on_complete;
};

class mpi3_interface
    : public virtual mpi::mpi_interface
{
public:
    mpi3_interface(endpoint& ep)
        : mpi::mpi_interface(ep) { }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_rget(const rget_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_rput(const rput_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_compare_and_swap(const compare_and_swap_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_fetch_and_op(const fetch_and_op_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_ibarrier(const ibarrier_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_ibcast(const ibcast_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_iallgather(const iallgather_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_ialltoall(const ialltoall_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_attach_async(const attach_async_params&) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_detach_async(const detach_async_params&) = 0;
    
    MPI_Aint attach(const attach_params& params)
    {
        mgbase::ult::sync_flag flag;
        
        MPI_Aint result;
        
        while (MGBASE_UNLIKELY(
            !try_attach_async({ params, &result, mgbase::make_callback_notify(&flag) })
        )) {
            mgbase::ult::this_thread::yield();
        }
        
        flag.wait();
        
        return result;
    }
    
    void detach(const detach_params& params)
    {
        mgbase::ult::sync_flag flag;
        
        while (MGBASE_UNLIKELY(
            !try_detach_async({ params, mgbase::make_callback_notify(&flag) })
        )) {
            mgbase::ult::this_thread::yield();
        }
        
        flag.wait();
    }
};

} // namespace mpi3
} // namespace mgcom

