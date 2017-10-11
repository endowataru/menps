
#include <mgdev/ucx/ucp/context.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void context_deleter::operator () (ucp_context* const p) const MGBASE_NOEXCEPT
{
    ucp_cleanup(p);
}

context init(
    const ucp_params_t* const   params
,   const ucp_config_t* const   config
) {
    ucp_context* p = MGBASE_NULLPTR;
    
    const auto ret = ucp_init(params, config, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_init() failed", ret);
    }
    
    return context(p);
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

