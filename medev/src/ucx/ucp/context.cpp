
#include <menps/medev/ucx/ucp/context.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

void context_deleter::operator () (ucp_context* const p) const noexcept
{
    ucp_cleanup(p);
}

context init(
    const ucp_params_t* const   params
,   const ucp_config_t* const   config
) {
    ucp_context* p = nullptr;
    
    const auto ret = ucp_init(params, config, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_init() failed", ret);
    }
    
    return context(p);
}

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

