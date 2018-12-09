
#include <menps/medev/ucx/ucp/config.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

void config_deleter::operator () (ucp_config* const p) const noexcept
{
    ucp_config_release(p);
}

config read_config(
    const char* const   env_prefix
,   const char* const   filename
) {
    ucp_config* p = nullptr;
    
    const auto ret = ucp_config_read(env_prefix, filename, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_config_read() failed", ret);
    }
    
    return config(p);
}

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

