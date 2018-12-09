
#include <menps/medev/mpi/mpi1_requester.hpp>
#include <menps/medev/mpi/mpi_error.hpp>
#include <menps/medev/ult.hpp>

namespace menps {
namespace medev {
namespace mpi {

namespace /*unnamed*/ {

template <
    typename Params
,   typename AsyncParams
,   ult::async_status<void> (mpi1_requester::*Method)(AsyncParams)
>
struct async_closure
{
    mpi1_requester* self;
    const Params*   p;
    
    template <typename Cont>
    MEFDN_NODISCARD
    ult::async_status<void> operator() (Cont&& cont) const /*may throw*/
    {
        return (self->*Method)({ *p, mefdn::forward<Cont>(cont) });
    }
};

} // unnamed namespace

void mpi1_requester::recv(const recv_params params)
{
    ult::suspend_and_call<void>(
        async_closure<recv_params, recv_async_params, &mpi1_requester::recv_async>{ this, &params }
    );
}
void mpi1_requester::send(const send_params params)
{
    ult::suspend_and_call<void>(
        async_closure<send_params, send_async_params, &mpi1_requester::send_async>{ this, &params }
    );
}

void mpi1_requester::barrier(const barrier_params params)
{
    ult::suspend_and_call<void>(
        async_closure<barrier_params, barrier_async_params, &mpi1_requester::barrier_async>{ this, &params }
    );
}
void mpi1_requester::broadcast(const broadcast_params params)
{
    ult::suspend_and_call<void>(
        async_closure<broadcast_params, broadcast_async_params, &mpi1_requester::broadcast_async>{ this, &params }
    );
}
void mpi1_requester::allgather(const allgather_params params)
{
    ult::suspend_and_call<void>(
        async_closure<allgather_params, allgather_async_params, &mpi1_requester::allgather_async>{ this, &params }
    );
}
void mpi1_requester::alltoall(const alltoall_params params)
{
    ult::suspend_and_call<void>(
        async_closure<alltoall_params, alltoall_async_params, &mpi1_requester::alltoall_async>{ this, &params }
    );
}

int mpi1_requester::get_current_rank(const MPI_Comm comm)
{
    int ret;
    mpi_error::check(
        MPI_Comm_rank(comm, &ret)
    );
    return ret;
}

int mpi1_requester::get_num_ranks(const MPI_Comm comm)
{
    int ret;
    mpi_error::check(
        MPI_Comm_size(comm, &ret)
    );
    return ret;
}

} // namespace mpi
} // namespace medev
} // namespace menps

