
#include <mgdev/ucx/ucp/config.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void config_deleter::operator () (ucp_config* const p) const MGBASE_NOEXCEPT
{
    ucp_config_release(p);
}

config read_config(
    const char* const   env_prefix
,   const char* const   filename
) {
    ucp_config* p = MGBASE_NULLPTR;
    
    const auto ret = ucp_config_read(env_prefix, filename, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_config_read() failed", ret);
    }
    
    return config(p);
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

