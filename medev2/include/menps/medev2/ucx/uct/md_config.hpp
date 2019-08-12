
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct md_config_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator() (uct_md_config_t* const p) const noexcept {
        uf->config_release({ p });
    }
};

template <typename P>
class md_config
    : public mefdn::unique_ptr<uct_md_config_t, md_config_deleter<P>>
{
    using deleter_type = md_config_deleter<P>;
    using base = mefdn::unique_ptr<uct_md_config_t, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    md_config() noexcept = default;
    
    explicit md_config(
        uct_facade_type&        uf
    ,   uct_md_config_t* const  p
    )
        : base(p, deleter_type{ &uf })
    { }
    
    md_config(const md_config&) = delete;
    md_config& operator = (const md_config&) = delete;
    
    md_config(md_config&&) noexcept = default;
    md_config& operator = (md_config&&) noexcept = default;
    
    static md_config read(
        uct_facade_type&    uf
    ,   const char* const   md_name
    ) {
        uct_md_config_t* p = nullptr;
        
        const auto ret = uf.md_config_read({ md_name, nullptr, nullptr, &p });
        if (ret != UCS_OK) {
            throw ucx_error("uct_md_config_read() failed", ret);
        }
        
        return md_config(uf, p);
    }
};

} // namespace uct
} // namesapce ucx
} // namespace medev2
} // namespace menps

