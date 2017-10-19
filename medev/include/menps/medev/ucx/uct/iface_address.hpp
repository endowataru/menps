
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct iface_address_deleter
{
    void operator () (uct_iface_addr_t* const p) const noexcept
    {
        delete [] reinterpret_cast<mefdn::uint8_t*>(p);
    }
};

class iface_address
    : public mefdn::unique_ptr<uct_iface_addr_t, iface_address_deleter>
{
    typedef mefdn::unique_ptr<uct_iface_addr_t, iface_address_deleter>  base;
    
public:
    iface_address() noexcept = default;
    
    explicit iface_address(uct_iface_addr_t* const p)
        : base(p)
    { }
    
    iface_address(const iface_address&) = delete;
    iface_address& operator = (const iface_address&) = delete;
    
    iface_address(iface_address&&) noexcept = default;
    iface_address& operator = (iface_address&&) noexcept = default;
};

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

