
#include <mgdev/mpi/mpi3_requester.hpp>
#include <mgdev/ult.hpp>

namespace mgdev {
namespace mpi {

namespace /*unnamed*/ {

template <
    typename Params
,   typename AsyncParams
,   ult::async_status<void> (mpi3_requester::*Method)(AsyncParams)
>
struct async_closure
{
    mpi3_requester* self;
    const Params*   p;
    
    template <typename Cont>
    MGBASE_WARN_UNUSED_RESULT
    ult::async_status<void> operator() (Cont& cont) const /*may throw*/
    {
        return (self->*Method)({ *p, cont });
    }
};

} // unnamed namespace

void mpi3_requester::get(const get_params params)
{
    ult::suspend_and_call<void>(
        async_closure<get_params, get_async_params, &mpi3_requester::get_async>{ this, &params }
    );
}

void mpi3_requester::put(const put_params params)
{
    ult::suspend_and_call<void>(
        async_closure<put_params, put_async_params, &mpi3_requester::put_async>{ this, &params }
    );
}

void mpi3_requester::compare_and_swap(const compare_and_swap_params params)
{
    ult::suspend_and_call<void>(
        async_closure<compare_and_swap_params, compare_and_swap_async_params, &mpi3_requester::compare_and_swap_async>{ this, &params }
    );
}

void mpi3_requester::fetch_and_op(const fetch_and_op_params params)
{
    ult::suspend_and_call<void>(
        async_closure<fetch_and_op_params, fetch_and_op_async_params, &mpi3_requester::fetch_and_op_async>{ this, &params }
    );
}

} // namespace mpi
} // namespace mgdev

