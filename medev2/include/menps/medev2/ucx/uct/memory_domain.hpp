
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct memory_domain_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator () (uct_md* const p) const noexcept {
        uf->md_close({ p });
    }
};

template <typename P>
class memory_domain
    : public mefdn::unique_ptr<uct_md, memory_domain_deleter<P>>
{
    using deleter_type = memory_domain_deleter<P>;
    using base = mefdn::unique_ptr<uct_md, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    memory_domain() noexcept = default;
    
    explicit memory_domain(
        uct_facade_type&    uf
    ,   uct_md* const       p
    )
        : base(p, deleter_type{ &uf })
    { }
    
    memory_domain(const memory_domain&) = delete;
    memory_domain& operator = (const memory_domain&) = delete;
    
    memory_domain(memory_domain&&) noexcept = default;
    memory_domain& operator = (memory_domain&&) noexcept = default;
    
    static memory_domain open(
        uct_facade_type&        uf
    ,   const char* const       md_name
    ,   uct_md_config_t* const  conf
    ) {
        uct_md* md = nullptr;
        
        const auto ret = uf.md_open({ md_name, conf, &md });
        if (ret != UCS_OK) {
            throw ucx_error("uct_md_open() failed", ret);
        }
        
        return memory_domain(uf, md);
    }
    
    uct_md_attr_t query()
    {
        auto& uf = * this->get_deleter().uf;
        
        uct_md_attr_t attr = uct_md_attr_t();
        
        const auto ret = uf.md_query({ this->get(), &attr });
        if (ret != UCS_OK) {
            throw ucx_error("uct_md_query() failed", ret);
        }
        
        return attr;
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

