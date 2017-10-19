
#include <menps/medev/ucx/uct/iface_config.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void iface_config_deleter::operator () (uct_iface_config_t* const md) const noexcept
{
    uct_config_release(md);
}

iface_config read_iface_config(
    const char* const   tl_name
,   const char* const   env_prefix
,   const char* const   filename
) {
    uct_iface_config_t* p = nullptr;
    
    const auto ret = uct_iface_config_read(tl_name, env_prefix, filename, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_iface_config_read() failed", ret);
    }
    
    return iface_config(p);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

