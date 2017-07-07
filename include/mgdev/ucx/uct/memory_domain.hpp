
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct memory_domain_deleter
{
    void operator () (uct_md*) const MGBASE_NOEXCEPT;
};

class memory_domain
    : public mgbase::unique_ptr<uct_md, memory_domain_deleter>
{
    typedef mgbase::unique_ptr<uct_md, memory_domain_deleter>  base;
    
public:
    memory_domain() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit memory_domain(uct_md* const p)
        : base(p)
    { }
    
    memory_domain(const memory_domain&) = delete;
    memory_domain& operator = (const memory_domain&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(memory_domain, base)
};

memory_domain open_memory_domain(const char* md_name, uct_md_config_t*);

memory_domain open_memory_domain(uct_md_resource_desc_t* md_desc);

} // namespace uct
} // namespace ucx
} // namespace mgdev

