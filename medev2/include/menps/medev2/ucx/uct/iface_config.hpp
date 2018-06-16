
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct iface_config_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator () (uct_iface_config_t* const p) const noexcept {
        uf->config_release({ p });
    }
};

template <typename P>
class iface_config
    : public mefdn::unique_ptr<uct_iface_config_t, iface_config_deleter<P>>
{
    using deleter_type = iface_config_deleter<P>;
    using base = mefdn::unique_ptr<uct_iface_config_t, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    iface_config() noexcept = default;
    
    explicit iface_config(
        uct_facade_type&            uf
    ,   uct_iface_config_t* const   p
    )
        : base(p, deleter_type{ &uf })
    { }
    
    iface_config(const iface_config&) = delete;
    iface_config& operator = (const iface_config&) = delete;
    
    iface_config(iface_config&&) noexcept = default;
    iface_config& operator = (iface_config&&) noexcept = default;
    
    static iface_config read(
        uct_facade_type&    uf
    ,   uct_md_t* const     md
    ,   const char* const   tl_name
    ,   const char* const   env_prefix
    ,   const char* const   filename
    ) {
        uct_iface_config_t* p = nullptr;
        
        const auto ret =
            uf.md_iface_config_read({ md, tl_name, env_prefix, filename, &p });
        
        if (ret != UCS_OK) {
            throw ucx_error("uct_iface_config_read() failed", ret);
        }
        
        return iface_config(uf, p);
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

