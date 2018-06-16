
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct md_resource_list_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator() (uct_md_resource_desc_t* const p) const noexcept {
        uf->release_md_resource_list({ p });
    }
};

template <typename P>
class md_resource_list
    : public mefdn::unique_ptr<uct_md_resource_desc_t [], md_resource_list_deleter<P>>
{
    using deleter_type = md_resource_list_deleter<P>;
    using base = mefdn::unique_ptr<uct_md_resource_desc_t [], deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    md_resource_list() MEFDN_DEFAULT_NOEXCEPT = default;
    
    explicit md_resource_list(
        uct_facade_type&                uf
    ,   uct_md_resource_desc_t* const   p
    ,   const mefdn::size_t             size
    )
        : base(p, deleter_type{ &uf })
        , size_(size)
    { }
    
    md_resource_list(const md_resource_list&) = delete;
    md_resource_list& operator = (const md_resource_list&) = delete;
    
    md_resource_list(md_resource_list&&) noexcept = default;
    md_resource_list& operator = (md_resource_list&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return this->size_;
    }
    
    uct_md_resource_desc_t* begin() noexcept {
        return this->get();
    }
    uct_md_resource_desc_t* end() noexcept {
        return this->get() + this->size();
    }
    
    static md_resource_list query(uct_facade_type& uf)
    {
        uct_md_resource_desc_t* p = nullptr;
        unsigned int num = 0;
        
        const auto ret = uf.query_md_resources({ &p, &num });
        if (ret != UCS_OK) {
            throw ucx_error("uct_query_md_resources() failed", ret);
        }
        
        return md_resource_list(uf, p, num);
    }
    
private:
    mefdn::size_t size_ = 0;
};

} // namespace uct
} // namesapce ucx
} // namespace medev2
} // namespace menps

