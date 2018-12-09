
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct device_address_deleter
{
    void operator () (uct_device_addr_t* const p) const noexcept
    {
        delete [] reinterpret_cast<mefdn::uint8_t*>(p);
    }
};

class device_address
    : public mefdn::unique_ptr<uct_device_addr_t, device_address_deleter>
{
    typedef mefdn::unique_ptr<uct_device_addr_t, device_address_deleter>  base;
    
public:
    device_address() noexcept = default;
    
    explicit device_address(uct_device_addr_t* const p)
        : base(p)
    { }
    
    device_address(const device_address&) = delete;
    device_address& operator = (const device_address&) = delete;
    
    device_address(device_address&&) noexcept = default;
    device_address& operator = (device_address&&) noexcept = default;
};

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

