
#pragma once

#include <mgdev/ucx/uct/iface_address.hpp>
#include <mgdev/ucx/uct/device_address.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct interface_deleter
{
    void operator () (uct_iface*) const MGBASE_NOEXCEPT;
};

class interface
    : public mgbase::unique_ptr<uct_iface, interface_deleter>
{
    typedef mgbase::unique_ptr<uct_iface, interface_deleter>  base;
    
public:
    interface() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit interface(uct_iface* const p)
        : base(p)
    { }
    
    interface(const interface&) = delete;
    interface& operator = (const interface&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(interface, base)
    
    uct_iface_attr_t query();
    
    void get_address(uct_iface_addr_t*);
    
    iface_address get_address();
    
    void get_device_address(uct_device_addr_t*);
    
    device_address get_device_address();
    
    void set_am_handler(
        mgbase::uint8_t     id
    ,   uct_am_callback_t   cb
    ,   void*               arg
    ,   mgbase::uint32_t    flags
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
} // namespace mgdev

