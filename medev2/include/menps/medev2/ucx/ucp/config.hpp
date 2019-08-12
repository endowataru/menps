
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct config_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type* uf;
    
    void operator () (ucp_config* const conf) const noexcept {
        this->uf->config_release({ conf });
    }
};

template <typename P>
class config
    : public mefdn::unique_ptr<ucp_config, config_deleter<P>>
{
    using deleter_type = config_deleter<P>;
    using base = mefdn::unique_ptr<ucp_config, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    explicit config(ucp_facade_type& uf, ucp_config* const p)
        : base(p, deleter_type{ &uf })
    { }
   
    config(const config&) = delete;
    config& operator = (const config&) = delete;
    
    config(config&&) noexcept = default;
    config& operator = (config&&) noexcept = default;
    
    static config read(
        ucp_facade_type&       uf
    ,   const char* const   env_prefix
    ,   const char* const   filename
    ) {
        ucp_config* conf = nullptr;
        
        uf.config_read({ env_prefix, filename, &conf });
        
        return config(uf, conf);
    }
};

} // namespace ucp
} // namesapce ucx
} // namespace medev2
} // namespace menps

