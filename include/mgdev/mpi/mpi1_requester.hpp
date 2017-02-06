
#pragma once

#include <mgdev/mpi/mpi.hpp>
#include <mgbase/callback.hpp>

namespace mgdev {
namespace mpi {

struct recv_params
{
    void*       buf;
    int         num_bytes;
    int         src_rank;
    int         tag;
    MPI_Comm    comm;
    MPI_Status* status_result;
};

struct send_params
{
    const void* buf;
    int         num_bytes;
    int         dest_rank;
    int         tag;
    MPI_Comm    comm;
};

struct barrier_params
{
    MPI_Comm    comm;
};

struct broadcast_params
{
    int         root;
    void*       ptr;
    int         num_bytes;
    MPI_Comm    comm;
};

struct allgather_params
{
    const void* src;
    void*       dest;
    int         num_bytes;
    MPI_Comm    comm;
};

struct alltoall_params
{
    const void* src;
    void*       dest;
    int         num_bytes;
    MPI_Comm    comm;
};

struct recv_async_params
{
    recv_params                 base;
    mgbase::callback<void ()>   on_complete;
};

struct send_async_params
{
    send_params                 base;
    mgbase::callback<void ()>   on_complete;
};

struct barrier_async_params
{
    barrier_params              base;
    mgbase::callback<void ()>   on_complete;
};

struct broadcast_async_params
{
    broadcast_params            base;
    mgbase::callback<void ()>   on_complete;
};

struct allgather_async_params
{
    allgather_params            base;
    mgbase::callback<void ()>   on_complete;
};

struct alltoall_async_params
{
    alltoall_params             base;
    mgbase::callback<void ()>   on_complete;
};

class mpi1_requester
{
public:
    virtual ~mpi1_requester() /*noexcept*/ = default;
    
    // Point-to-point communication
    
    virtual void recv(recv_params);
    
    virtual void send(send_params);
    
    virtual void recv_async(recv_async_params) = 0;
    
    virtual void send_async(send_async_params) = 0;
    
    void recv(
        void* const                     buf
    ,   const int                       num_bytes
    ,   const int                       src_rank
    ,   const int                       tag
    ,   const MPI_Comm                  comm
    ,   MPI_Status* const               status_result
    ) {
        this->recv(recv_params{
            buf
        ,   num_bytes
        ,   src_rank
        ,   tag
        ,   comm
        ,   status_result
        });
    }
    
    void send(
        const void* const               buf
    ,   const int                       num_bytes
    ,   const int                       dest_rank
    ,   const int                       tag
    ,   const MPI_Comm                  comm
    ) {
        this->send(send_params{
            buf
        ,   num_bytes
        ,   dest_rank
        ,   tag
        ,   comm
        });
    }
    
    void recv_async(
        void* const                     buf
    ,   const int                       num_bytes
    ,   const int                       src_rank
    ,   const int                       tag
    ,   const MPI_Comm                  comm
    ,   MPI_Status* const               status_result
    ,   const mgbase::callback<void ()> on_complete
    ) {
        this->recv_async(recv_async_params{
            recv_params{
                buf
            ,   num_bytes
            ,   src_rank
            ,   tag
            ,   comm
            ,   status_result
            }
        ,   on_complete
        });
    }
    
    void send_async(
        const void* const               buf
    ,   const int                       num_bytes
    ,   const int                       dest_rank
    ,   const int                       tag
    ,   const MPI_Comm                  comm
    ,   const mgbase::callback<void ()> on_complete
    ) {
        this->send_async(send_async_params{
            send_params{
                buf
            ,   num_bytes
            ,   dest_rank
            ,   tag
            ,   comm
            }
        ,   on_complete
        });
    }
    
    // Collective communication
    
    virtual void barrier(barrier_params);
    
    virtual void broadcast(broadcast_params);
    
    virtual void allgather(allgather_params);
    
    virtual void alltoall(alltoall_params);
    
    virtual void barrier_async(barrier_async_params) = 0;
    
    virtual void broadcast_async(broadcast_async_params) = 0;
    
    virtual void allgather_async(allgather_async_params) = 0;
    
    virtual void alltoall_async(alltoall_async_params) = 0;
    
    // Communicators
    
    virtual MPI_Comm comm_dup(MPI_Comm) = 0;
    
    virtual void comm_free(MPI_Comm*) = 0;
    
    virtual void comm_set_name(MPI_Comm, const char*) = 0;
    
    MPI_Comm comm_dup(const MPI_Comm comm, const char* const comm_name)
    {
        const auto new_comm = comm_dup(comm);
        comm_set_name(new_comm, comm_name);
        return new_comm;
    }
    
    virtual int get_current_rank(MPI_Comm);
    
    virtual int get_num_ranks(MPI_Comm);
};

} // namespace mpi
} // namespace mgdev

