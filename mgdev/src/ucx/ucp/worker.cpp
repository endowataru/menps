
#include <mgdev/ucx/ucp/worker.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void worker_deleter::operator () (ucp_worker* const p) const MGBASE_NOEXCEPT
{
    ucp_worker_destroy(p);
}

worker create_worker(
    ucp_context* const                  ctx
,   const ucp_worker_params_t* const    params
) {
    ucp_worker* p = MGBASE_NULLPTR;
    
    const auto ret = ucp_worker_create(ctx, params, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_worker_create() failed", ret);
    }
    
    return worker(p);
}

worker_address worker::get_address()
{
    return get_worker_address(this->get());
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

