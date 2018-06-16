
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct interface_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator () (uct_iface* const p) const noexcept {
        uf->iface_close({ p });
    }
};

template <typename P>
class interface
    : public mefdn::unique_ptr<uct_iface, interface_deleter<P>>
{
    using deleter_type = interface_deleter<P>;
    using base = mefdn::unique_ptr<uct_iface, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    interface() noexcept = default;
    
    explicit interface(
        uct_facade_type&    uf
    ,   uct_iface* const    p
    )
        : base(p, deleter_type{ &uf })
    { }
    
    interface(const interface&) = delete;
    interface& operator = (const interface&) = delete;
    
    interface(interface&&) noexcept = default;
    interface& operator = (interface&&) noexcept = default;
    
    static interface open(
        uct_facade_type&                    uf
    ,   uct_md* const                       md
    ,   uct_worker* const                   wk
    ,   const uct_iface_params_t* const     params
    ,   const uct_iface_config_t* const     config
    ) {
        uct_iface* p = nullptr;
        
        const auto ret = uf.iface_open({ md, wk, params, config, &p });
        if (ret != UCS_OK) {
            throw ucx_error("uct_iface_open() failed", ret);
        }
        
        return interface(uf, p);
    }
    
    uct_iface_attr_t query()
    {
        auto& uf = * this->get_deleter().uf;
        
        uct_iface_attr_t attr = uct_iface_attr_t();
        
        const auto ret = uf.iface_query({ this->get(), &attr });
        if (ret != UCS_OK) {
            throw ucx_error("uct_iface_query() failed", ret);
        }
        
        return attr;
    }
    
    void get_iface_address(uct_iface_addr_t* const addr)
    {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret = uf.iface_get_address({ this->get(), addr });
        if (ret != UCS_OK) {
            throw ucx_error("uct_iface_get_address() failed", ret);
        }
    }
    
    void get_device_address(uct_device_addr_t* const addr)
    {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret = uf.iface_get_device_address({ this->get(), addr });
        if (ret != UCS_OK) {
            throw ucx_error("uct_iface_get_device_address() failed", ret);
        }
    }
    
    unsigned int progress()
    {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret = uf.iface_progress({ this->get() });
        return ret;
    }
    
    #if 0
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
    #endif
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

