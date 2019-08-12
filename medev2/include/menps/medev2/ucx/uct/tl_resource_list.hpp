
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct tl_resource_list_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator() (uct_tl_resource_desc_t* const p) const noexcept {
        uf->release_tl_resource_list({ p });
    }
};

template <typename P>
class tl_resource_list
    : public mefdn::unique_ptr<uct_tl_resource_desc_t [], tl_resource_list_deleter<P>>
{
    using deleter_type = tl_resource_list_deleter<P>;
    using base = mefdn::unique_ptr<uct_tl_resource_desc_t [], deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    tl_resource_list() MEFDN_DEFAULT_NOEXCEPT = default;
    
    explicit tl_resource_list(
        uct_facade_type&                uf
    ,   uct_tl_resource_desc_t* const   p
    ,   const mefdn::size_t             size
    )
        : base(p, deleter_type{ &uf })
        , size_(size)
    { }
    
    tl_resource_list(const tl_resource_list&) = delete;
    tl_resource_list& operator = (const tl_resource_list&) = delete;
    
    tl_resource_list(tl_resource_list&&) noexcept = default;
    tl_resource_list& operator = (tl_resource_list&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return this->size_;
    }
    
    uct_tl_resource_desc_t* begin() noexcept {
        return this->get();
    }
    uct_tl_resource_desc_t* end() noexcept {
        return this->get() + this->size();
    }
    
    static tl_resource_list query(
        uct_facade_type&    uf
    ,   uct_md* const       md
    ) {
        uct_tl_resource_desc_t* p = nullptr;
        unsigned int num = 0;
        
        const auto ret = uf.md_query_tl_resources({ md, &p, &num });
        if (ret != UCS_OK) {
            throw ucx_error("uct_query_tl_resources() failed", ret);
        }
        
        return tl_resource_list(uf, p, num);
    }
    
private:
    mefdn::size_t size_ = 0;
};

} // namespace uct
} // namesapce ucx
} // namespace medev2
} // namespace menps

