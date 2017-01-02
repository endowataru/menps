
#include <mgdev/mpi/mpi1_requester.hpp>
#include <mgdev/mpi/mpi_error.hpp>
#include <mgdev/ult.hpp>

namespace mgdev {
namespace mpi {

void mpi1_requester::recv(const recv_params params)
{
    ult::sync_flag f;
    
    this->recv_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}
void mpi1_requester::send(const send_params params)
{
    ult::sync_flag f;
    
    this->send_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}

void mpi1_requester::barrier(const barrier_params params)
{
    ult::sync_flag f;
    
    this->barrier_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}
void mpi1_requester::broadcast(const broadcast_params params)
{
    ult::sync_flag f;
    
    this->broadcast_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}
void mpi1_requester::allgather(const allgather_params params)
{
    ult::sync_flag f;
    
    this->allgather_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}
void mpi1_requester::alltoall(const alltoall_params params)
{
    ult::sync_flag f;
    
    this->alltoall_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}

int mpi1_requester::get_current_rank(const MPI_Comm comm)
{
    int ret;
    mpi_error::check(
        MPI_Comm_rank(MPI_COMM_WORLD, &ret)
    );
    return ret;
}

int mpi1_requester::get_num_ranks(const MPI_Comm comm)
{
    int ret;
    mpi_error::check(
        MPI_Comm_size(MPI_COMM_WORLD, &ret)
    );
    return ret;
}

} // namespace mpi
} // namespace mgdev

