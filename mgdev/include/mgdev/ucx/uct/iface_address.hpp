
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct iface_address_deleter
{
    void operator () (uct_iface_addr_t* const p) const MGBASE_NOEXCEPT
    {
        delete [] reinterpret_cast<mgbase::uint8_t*>(p);
    }
};

class iface_address
    : public mgbase::unique_ptr<uct_iface_addr_t, iface_address_deleter>
{
    typedef mgbase::unique_ptr<uct_iface_addr_t, iface_address_deleter>  base;
    
public:
    iface_address() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit iface_address(uct_iface_addr_t* const p)
        : base(p)
    { }
    
    iface_address(const iface_address&) = delete;
    iface_address& operator = (const iface_address&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(iface_address, base)
};

} // namespace uct
} // namespace ucx
} // namespace mgdev

