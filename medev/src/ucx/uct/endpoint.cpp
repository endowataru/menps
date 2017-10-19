
#include <menps/medev/ucx/uct/endpoint.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void endpoint_deleter::operator () (uct_ep* const p) const noexcept
{
    uct_ep_destroy(p);
}

endpoint create_endpoint(uct_iface* const iface)
{
    uct_ep* p = nullptr;
    
    const auto ret = uct_ep_create(iface, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_ep_create() failed", ret);
    }
    
    return endpoint(p);
}

endpoint create_endpoint_connected(
    uct_iface_h                     iface
,   const uct_device_addr_t* const  dev_addr
,   const uct_iface_addr_t* const   iface_addr
) {
    uct_ep* p = nullptr;
    
    const auto ret = uct_ep_create_connected(iface, dev_addr, iface_addr, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_ep_create_connected() failed", ret);
    }
    
    return endpoint(p);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

