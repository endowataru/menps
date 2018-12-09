
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct tl_resource_list_deleter
{
    void operator() (uct_tl_resource_desc_t*) const noexcept;
};

class tl_resource_list
    : public mefdn::unique_ptr<uct_tl_resource_desc_t [], tl_resource_list_deleter>
{
    typedef mefdn::unique_ptr<uct_tl_resource_desc_t [], tl_resource_list_deleter>  base;
    
public:
    tl_resource_list() noexcept = default;
    
    explicit tl_resource_list(uct_tl_resource_desc_t* const p, const mefdn::size_t size)
        : base(p)
        , size_(size)
    { }
    
    tl_resource_list(const tl_resource_list&) = delete;
    tl_resource_list& operator = (const tl_resource_list&) = delete;
    
    tl_resource_list(tl_resource_list&&) noexcept = default;
    tl_resource_list& operator = (tl_resource_list&&) noexcept = default;
    
    mefdn::size_t size() const noexcept {
        return size_;
    }
    
    uct_tl_resource_desc_t* begin() noexcept {
        return this->get();
    }
    uct_tl_resource_desc_t* end() noexcept {
        return this->get() + this->size();
    }
    
    uct_tl_resource_desc_t* get_by_name(const char* name) const;
    
private:
    mefdn::size_t size_;
};

tl_resource_list query_tl_resources(uct_md*);

} // namespace uct
} // namesapce ucx
} // namespace medev
} // namespace menps

