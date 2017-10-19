
#include <menps/medev/ucx/ucp/endpoint.hpp>
#include <menps/medev/ucx/ucx_error.hpp>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

void endpoint_deleter::operator () (ucp_ep* const p) const noexcept
{
    // TODO : What will happen with "non-blocking disconnect"?
    ucp_disconnect_nb(p);
}

endpoint create_endpoint(
    ucp_worker*             wk
,   const ucp_ep_params_t*  params
) {
    ucp_ep* p = nullptr;
    
    const auto ret = ucp_ep_create(wk, params, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_ep_create() failed", ret);
    }
    
    return endpoint(p);
}

void endpoint::throw_error(const char* const func_name, const ucs_status_t status)
{
    throw ucx_error(fmt::format("{}() failed", func_name), status);
}

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

