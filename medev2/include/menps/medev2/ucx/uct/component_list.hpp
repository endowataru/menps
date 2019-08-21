
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct component_list_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator() (uct_component** const p) const noexcept {
        uf->release_component_list({ p });
    }
};

template <typename P>
class component_list
    : public mefdn::unique_ptr<uct_component* [], component_list_deleter<P>>
{
    using deleter_type = component_list_deleter<P>;
    using base = mefdn::unique_ptr<uct_component* [], deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    component_list() MEFDN_DEFAULT_NOEXCEPT = default;
    
    explicit component_list(
        uct_facade_type&        uf
    ,   uct_component** const   p
    ,   const mefdn::size_t     size
    )
        : base(p, deleter_type{ &uf })
        , size_(size)
    { }
    
    component_list(const component_list&) = delete;
    component_list& operator = (const component_list&) = delete;
    
    component_list(component_list&&) noexcept = default;
    component_list& operator = (component_list&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return this->size_;
    }
    
    uct_component** begin() noexcept {
        return this->get();
    }
    uct_component** end() noexcept {
        return this->get() + this->size();
    }
    
    static component_list query(uct_facade_type& uf)
    {
        uct_component** p = nullptr;
        unsigned int num = 0;
        
        const auto ret = uf.query_components({ &p, &num });
        if (ret != UCS_OK) {
            throw ucx_error("uct_query_md_resources() failed", ret);
        }
        
        return component_list(uf, p, num);
    }
    
    // TODO: same function name
    void query(
        uct_component* const        component
    ,   uct_component_attr_t* const attr
    ) {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret = uf.component_query({ component, attr });
        if (ret != UCS_OK) {
            throw ucx_error("uct_component_query() failed", ret);
        }
    }
    
    unsigned int query_md_resource_count(uct_component* const component)
    {
        uct_component_attr_t attr = uct_component_attr_t();
        attr.field_mask = UCT_COMPONENT_ATTR_FIELD_MD_RESOURCE_COUNT;
        
        this->query(component, &attr);
        
        return attr.md_resource_count;
    }
    
    void query_md_resources(
        uct_component* const            component
    ,   uct_md_resource_desc_t* const   md_resources
    ) {
        uct_component_attr_t attr = uct_component_attr_t();
        attr.field_mask = UCT_COMPONENT_ATTR_FIELD_MD_RESOURCES;
        attr.md_resources = md_resources;
        
        this->query(component, &attr);
    }
    
private:
    mefdn::size_t size_ = 0;
};

} // namespace uct
} // namesapce ucx
} // namespace medev2
} // namespace menps

