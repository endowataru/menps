
#pragma once

#include <menps/medev/ucx/uct/iface_address.hpp>
#include <menps/medev/ucx/uct/device_address.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct interface_deleter
{
    void operator () (uct_iface*) const noexcept;
};

class interface
    : public mefdn::unique_ptr<uct_iface, interface_deleter>
{
    typedef mefdn::unique_ptr<uct_iface, interface_deleter>  base;
    
public:
    interface() noexcept = default;
    
    explicit interface(uct_iface* const p)
        : base(p)
    { }
    
    interface(const interface&) = delete;
    interface& operator = (const interface&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(interface, base)
    
    uct_iface_attr_t query();
    
    void get_address(uct_iface_addr_t*);
    
    iface_address get_address();
    
    void get_device_address(uct_device_addr_t*);
    
    device_address get_device_address();
    
    void set_am_handler(
        mefdn::uint8_t     id
    ,   uct_am_callback_t   cb
    ,   void*               arg
    ,   mefdn::uint32_t    flags
    );
};

interface open_interface(
    uct_md*                     md
,   uct_worker*                 wk
,   const uct_iface_params_t*   params
,   const uct_iface_config_t*   config
);

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

