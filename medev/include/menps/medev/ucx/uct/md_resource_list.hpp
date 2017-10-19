
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct md_resource_list_deleter
{
    void operator() (uct_md_resource_desc_t*) const noexcept;
};

class md_resource_list
    : public mefdn::unique_ptr<uct_md_resource_desc_t [], md_resource_list_deleter>
{
    typedef mefdn::unique_ptr<uct_md_resource_desc_t [], md_resource_list_deleter>  base;
    
public:
    md_resource_list() noexcept = default;
    
    explicit md_resource_list(uct_md_resource_desc_t* const p, const mefdn::size_t size)
        : base(p)
        , size_(size)
    { }
    
    md_resource_list(const md_resource_list&) = delete;
    md_resource_list& operator = (const md_resource_list&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(md_resource_list, base, size_)
    
    mefdn::size_t size() const noexcept {
        return size_;
    }
    
    uct_md_resource_desc_t* begin() noexcept {
        return this->get();
    }
    uct_md_resource_desc_t* end() noexcept {
        return this->get() + this->size();
    }
    
    /*
    uct_md_resource_desc_t* get_by_name(const char* tl_name, const char* dev_name) const;
    
    uct_md_resource_desc_t* get_by_name(const char* name) const;*/
    
private:
    mefdn::size_t size_;
};

md_resource_list query_md_resources();

} // namespace uct
} // namesapce ucx
} // namespace medev
} // namespace menps

