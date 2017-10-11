
#include <mgdev/ucx/uct/interface.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

void interface_deleter::operator () (uct_iface* const md) const MGBASE_NOEXCEPT
{
    uct_iface_close(md);
}

interface open_interface(
    uct_md* const                       md
,   uct_worker* const                   wk
,   const uct_iface_params_t* const     params
,   const uct_iface_config_t* const     config
) {
    uct_iface* p = MGBASE_NULLPTR;
    
    const auto ret = uct_iface_open(md, wk, params, config, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_iface_open() failed", ret);
    }
    
    return interface(p);
}

uct_iface_attr_t interface::query()
{
    uct_iface_attr_t attr = uct_iface_attr_t();
    
    const auto ret = uct_iface_query(this->get(), &attr);
    if (ret != UCS_OK) {
        throw ucx_error("uct_iface_query() failed", ret);
    }
    
    return attr;
}

void interface::get_address(uct_iface_addr_t* const addr)
{
    const auto ret = uct_iface_get_address(this->get(), addr);
    if (ret != UCS_OK) {
        throw ucx_error("uct_iface_get_address() failed", ret);
    }
}

iface_address interface::get_address()
{
    const auto iface_attr = this->query();
    
    iface_address addr(
        reinterpret_cast<uct_iface_addr_t*>(
            // Note: default initialization by parenthesis
            new mgbase::uint8_t[iface_attr.iface_addr_len]()
        )
    );
    
    this->get_address(addr.get());
    
    return addr;
}

void interface::get_device_address(uct_device_addr_t* const addr)
{
    const auto ret = uct_iface_get_device_address(this->get(), addr);
    if (ret != UCS_OK) {
        throw ucx_error("uct_device_get_address() failed", ret);
    }
}

device_address interface::get_device_address()
{
    const auto device_attr = this->query();
    
    device_address addr(
        reinterpret_cast<uct_device_addr_t*>(
            // Note: default initialization by parenthesis
            new mgbase::uint8_t[device_attr.device_addr_len]()
        )
    );
    
    this->get_device_address(addr.get());
    
    return addr;
}

void interface::set_am_handler(
    const am_id_t           id
,   const uct_am_callback_t cb
,   void* const             arg
,   const mgbase::uint32_t  flags
) {
    const auto ret = uct_iface_set_am_handler(this->get(), id, cb, arg, flags);
    if (ret != UCS_OK) {
        throw ucx_error("uct_iface_set_am_handler() failed", ret);
    }
}

} // namespace uct
} // namespace ucx
} // namespace mgdev

