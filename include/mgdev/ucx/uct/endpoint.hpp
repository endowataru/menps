
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct endpoint_deleter
{
    void operator () (uct_ep*) const MGBASE_NOEXCEPT;
};

class endpoint
    : public mgbase::unique_ptr<uct_ep, endpoint_deleter>
{
    typedef mgbase::unique_ptr<uct_ep, endpoint_deleter>  base;
    
public:
    endpoint() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit endpoint(uct_ep* const p)
        : base(p)
    { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(endpoint, base)
    
    void am_short();
};

endpoint create_endpoint(uct_iface* iface);

endpoint create_endpoint_connected(
    uct_iface_h                 iface
,   const uct_device_addr_t*    dev_addr
,   const uct_iface_addr_t*     iface_addr
);

} // namespace uct
} // namespace ucx
} // namespace mgdev

