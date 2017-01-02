
#include <mgdev/mpi/mpi3_requester.hpp>
#include <mgdev/ult.hpp>

namespace mgdev {
namespace mpi {

void mpi3_requester::get(const get_params params)
{
    ult::sync_flag f;
    
    this->get_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}

void mpi3_requester::put(const put_params params)
{
    ult::sync_flag f;
    
    this->put_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}

void mpi3_requester::compare_and_swap(const compare_and_swap_params params)
{
    ult::sync_flag f;
    
    this->compare_and_swap_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}

void mpi3_requester::fetch_and_op(const fetch_and_op_params params)
{
    ult::sync_flag f;
    
    this->fetch_and_op_async({
        params
    ,   mgbase::make_callback_notify(&f)
    });
    
    f.wait();
}

} // namespace mpi
} // namespace mgdev

