
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct device_address_deleter
{
    void operator () (uct_device_addr_t* const p) const MGBASE_NOEXCEPT
    {
        delete [] reinterpret_cast<mgbase::uint8_t*>(p);
    }
};

class device_address
    : public mgbase::unique_ptr<uct_device_addr_t, device_address_deleter>
{
    typedef mgbase::unique_ptr<uct_device_addr_t, device_address_deleter>  base;
    
public:
    device_address() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit device_address(uct_device_addr_t* const p)
        : base(p)
    { }
    
    device_address(const device_address&) = delete;
    device_address& operator = (const device_address&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(device_address, base)
};

} // namespace uct
} // namespace ucx
} // namespace mgdev

