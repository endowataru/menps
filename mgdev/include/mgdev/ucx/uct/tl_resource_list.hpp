
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct tl_resource_list_deleter
{
    void operator() (uct_tl_resource_desc_t*) const MGBASE_NOEXCEPT;
};

class tl_resource_list
    : public mgbase::unique_ptr<uct_tl_resource_desc_t [], tl_resource_list_deleter>
{
    typedef mgbase::unique_ptr<uct_tl_resource_desc_t [], tl_resource_list_deleter>  base;
    
public:
    tl_resource_list() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit tl_resource_list(uct_tl_resource_desc_t* const p, const mgbase::size_t size)
        : base(p)
        , size_(size)
    { }
    
    tl_resource_list(const tl_resource_list&) = delete;
    tl_resource_list& operator = (const tl_resource_list&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(tl_resource_list, base, size_)
    
    mgbase::size_t size() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    uct_tl_resource_desc_t* begin() MGBASE_NOEXCEPT {
        return this->get();
    }
    uct_tl_resource_desc_t* end() MGBASE_NOEXCEPT {
        return this->get() + this->size();
    }
    
    uct_tl_resource_desc_t* get_by_name(const char* name) const;
    
private:
    mgbase::size_t size_;
};

tl_resource_list query_tl_resources(uct_md*);

} // namespace uct
} // namesapce ucx
} // namespace mgdev

