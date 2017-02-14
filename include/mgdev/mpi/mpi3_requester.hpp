
#pragma once

#include <mgdev/mpi/mpi1_requester.hpp>

namespace mgdev {
namespace mpi {

struct get_params
{
    void*       dest_ptr;
    int         src_rank;
    MPI_Aint    src_index;
    int         num_bytes;
    MPI_Win     win;
};

struct put_params
{
    const void* src_ptr;
    int         dest_rank;
    MPI_Aint    dest_index;
    int         num_bytes;
    MPI_Win     win;
};

struct compare_and_swap_params
{
    mgbase::uint64_t    expected;
    mgbase::uint64_t    desired;
    void*               result_ptr;
    MPI_Datatype        datatype;
    int                 dest_rank;
    MPI_Aint            dest_index;
    MPI_Win             win;
};

struct fetch_and_op_params
{
    mgbase::uint64_t    value;
    void*               result_ptr;
    MPI_Datatype        datatype;
    int                 dest_rank;
    MPI_Aint            dest_index;
    MPI_Op              operation;
    MPI_Win             win;
};

struct get_async_params
{
    get_params                  base;
    mgbase::callback<void ()>   on_complete;
};

struct put_async_params
{
    put_params                  base;
    mgbase::callback<void ()>   on_complete;
};

struct compare_and_swap_async_params
{
    compare_and_swap_params     base;
    mgbase::callback<void ()>   on_complete;
};

struct fetch_and_op_async_params
{
    fetch_and_op_params         base;
    mgbase::callback<void ()>   on_complete;
};

struct attach_params
{
    void*       ptr;
    MPI_Aint    size;
    MPI_Win     win;
};

struct detach_params
{
    void*   ptr;
    MPI_Win win;
};

class mpi3_requester
    : public virtual mpi1_requester
{
public:
    virtual void get(get_params);
    
    virtual void put(put_params);
    
    virtual void compare_and_swap(compare_and_swap_params);
    
    virtual void fetch_and_op(fetch_and_op_params);
    
    virtual ult::async_status<void> get_async(get_async_params)
        MGBASE_WARN_UNUSED_RESULT = 0;
    
    virtual ult::async_status<void> put_async(put_async_params)
        MGBASE_WARN_UNUSED_RESULT = 0;
    
    virtual ult::async_status<void> compare_and_swap_async(compare_and_swap_async_params)
        MGBASE_WARN_UNUSED_RESULT = 0;
    
    virtual ult::async_status<void> fetch_and_op_async(fetch_and_op_async_params)
        MGBASE_WARN_UNUSED_RESULT = 0;
    
    virtual MPI_Aint attach(attach_params) = 0;
    
    virtual void detach(detach_params) = 0;
};

} // namespace mpi
} // namespace mgdev

