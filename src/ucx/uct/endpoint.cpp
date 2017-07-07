
#include <mgdev/ucx/uct/endpoint.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

void endpoint_deleter::operator () (uct_ep* const p) const MGBASE_NOEXCEPT
{
    uct_ep_destroy(p);
}

endpoint create_endpoint(uct_iface* const iface)
{
    uct_ep* p = MGBASE_NULLPTR;
    
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
    uct_ep* p = MGBASE_NULLPTR;
    
    const auto ret = uct_ep_create_connected(iface, dev_addr, iface_addr, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_ep_create_connected() failed", ret);
    }
    
    return endpoint(p);
}

} // namespace uct
} // namespace ucx
} // namespace mgdev

