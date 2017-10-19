
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct endpoint_deleter
{
    void operator () (uct_ep*) const noexcept;
};

class endpoint
    : public mefdn::unique_ptr<uct_ep, endpoint_deleter>
{
    typedef mefdn::unique_ptr<uct_ep, endpoint_deleter>  base;
    
public:
    endpoint() noexcept = default;
    
    explicit endpoint(uct_ep* const p)
        : base(p)
    { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    endpoint(endpoint&&) noexcept = default;
    endpoint& operator = (endpoint&&) noexcept = default;
    
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
} // namespace medev
} // namespace menps

