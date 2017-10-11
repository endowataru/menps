
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct md_resource_list_deleter
{
    void operator() (uct_md_resource_desc_t*) const MGBASE_NOEXCEPT;
};

class md_resource_list
    : public mgbase::unique_ptr<uct_md_resource_desc_t [], md_resource_list_deleter>
{
    typedef mgbase::unique_ptr<uct_md_resource_desc_t [], md_resource_list_deleter>  base;
    
public:
    md_resource_list() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit md_resource_list(uct_md_resource_desc_t* const p, const mgbase::size_t size)
        : base(p)
        , size_(size)
    { }
    
    md_resource_list(const md_resource_list&) = delete;
    md_resource_list& operator = (const md_resource_list&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(md_resource_list, base, size_)
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    uct_md_resource_desc_t* begin() MGBASE_NOEXCEPT {
        return this->get();
    }
    uct_md_resource_desc_t* end() MGBASE_NOEXCEPT {
        return this->get() + this->size();
    }
    
    /*
    uct_md_resource_desc_t* get_by_name(const char* tl_name, const char* dev_name) const;
    
    uct_md_resource_desc_t* get_by_name(const char* name) const;*/
    
private:
    mgbase::size_t size_;
};

md_resource_list query_md_resources();

} // namespace uct
} // namesapce ucx
} // namespace mgdev

