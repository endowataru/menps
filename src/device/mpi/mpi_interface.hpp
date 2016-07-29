
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/collective/requester.hpp>
#include "mpi.hpp"

#include <mgbase/ult.hpp>

namespace mgcom {
namespace mpi {

struct irecv_params
{
    void*                       buf;
    int                         size_in_bytes;
    int                         source_rank;
    int                         tag;
    MPI_Comm                    comm;
    MPI_Status*                 status_result;
    mgbase::callback<void ()>   on_complete;
};

struct isend_params
{
    const void*                 buf;
    int                         size_in_bytes;
    int                         dest_rank;
    int                         tag;
    MPI_Comm                    comm;
    mgbase::callback<void ()>   on_complete;
};

struct barrier_params
{
    MPI_Comm            comm;
};

struct broadcast_params
{
    collective::untyped::broadcast_params   coll;
    MPI_Comm                                comm;
};

struct allgather_params
{
    collective::untyped::allgather_params   coll;
    MPI_Comm                                comm;
};

struct alltoall_params
{
    collective::untyped::alltoall_params    coll;
    MPI_Comm                                comm;
};

struct ibarrier_params
{
    barrier_params              base;
    mgbase::callback<void ()>   on_complete;
};

struct ibcast_params
{
    broadcast_params            base;
    mgbase::callback<void ()>   on_complete;
};

struct iallgather_params
{
    allgather_params            base;
    mgbase::callback<void ()>   on_complete;
};

struct ialltoall_params
{
    alltoall_params             base;
    mgbase::callback<void ()>   on_complete;
};

class mpi_interface
    : mgbase::noncopyable
{
protected:
    explicit mpi_interface(endpoint& ep) MGBASE_NOEXCEPT
        : ep_(ep) { }
    
public:
    virtual ~mpi_interface() MGBASE_EMPTY_DEFINITION
    
    endpoint& get_endpoint() MGBASE_NOEXCEPT { return ep_; }
    
    // non-blocking functions
    // these might fail
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_irecv(const irecv_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_isend(const isend_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_irsend(const isend_params& params) = 0;
    
    // ordinary non-blocking functions
    
    void irecv(const irecv_params& params)
    {
        while (MGBASE_UNLIKELY(!try_irecv(params)))
            mgbase::ult::this_thread::yield();
    }
    
    void isend(const isend_params& params)
    {
        while (MGBASE_UNLIKELY(!try_isend(params)))
            mgbase::ult::this_thread::yield();
    }
    
    void irsend(const isend_params& params)
    {
        while (MGBASE_UNLIKELY(!try_irsend(params)))
            mgbase::ult::this_thread::yield();
    }
    
    // communicator
    
    virtual MPI_Comm comm_dup(MPI_Comm comm) = 0;
    
    virtual void comm_free(MPI_Comm* comm) = 0;
    
    virtual void comm_set_name(MPI_Comm comm, const char* comm_name) = 0;
    
    MPI_Comm comm_dup(const MPI_Comm comm, const char* const comm_name)
    {
        const auto new_comm = comm_dup(comm);
        comm_set_name(new_comm, comm_name);
        return new_comm;
    }
    
    // collective
    
    /*
     * Important: This function blocks the communication thread.
     *            Therefore, it might cause serious deadlock problems.
     */
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_barrier_async(const ibarrier_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_broadcast_async(const ibcast_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_allgather_async(const iallgather_params& params) = 0;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_native_alltoall_async(const ialltoall_params& params) = 0;
    
    void native_barrier_async(const ibarrier_params& params)
    {
        while (MGBASE_UNLIKELY(!try_native_barrier_async(params)))
            mgbase::ult::this_thread::yield();
    }
    
    void native_broadcast_async(const ibcast_params& params)
    {
        while (MGBASE_UNLIKELY(!try_native_broadcast_async(params)))
            mgbase::ult::this_thread::yield();
    }
    
    void native_allgather_async(const iallgather_params& params)
    {
        while (MGBASE_UNLIKELY(!try_native_allgather_async(params)))
            mgbase::ult::this_thread::yield();
    }
    
    void native_alltoall_async(const ialltoall_params& params)
    {
        while (MGBASE_UNLIKELY(!try_native_alltoall_async(params)))
            mgbase::ult::this_thread::yield();  
    }
    
    void native_barrier(const barrier_params& params)
    {
        mgbase::ult::sync_flag flag;
        
        native_barrier_async({ params, mgbase::make_callback_notify(&flag) });
        
        flag.wait();
    }
    
    void native_broadcast(const broadcast_params& params)
    {
        mgbase::ult::sync_flag flag;
        
        native_broadcast_async({ params, mgbase::make_callback_notify(&flag) });
        
        flag.wait();
    }
    
    void native_alltoall(const alltoall_params& params)
    {
        mgbase::ult::sync_flag flag;
        
        native_alltoall_async({ params, mgbase::make_callback_notify(&flag) });
        
        flag.wait();
    }
    
    void native_allgather(const allgather_params& params)
    {
        mgbase::ult::sync_flag flag;
        
        native_allgather_async({ params, mgbase::make_callback_notify(&flag)  });
        
        flag.wait();
    }
    
private:
    endpoint& ep_;
};

template <typename T>
inline void native_alltoall(
    mpi_interface&  mi
,   const T* const  src
,   T* const        dest
,   const index_t   num_elems
) {
    mi.native_alltoall({ { src, dest, sizeof(T) * num_elems }, MPI_COMM_WORLD /*TODO*/ });
}

} // namespace mpi
} // namespace mgcom

