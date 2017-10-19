
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct memory_domain_deleter
{
    void operator () (uct_md*) const noexcept;
};

class memory_domain
    : public mefdn::unique_ptr<uct_md, memory_domain_deleter>
{
    typedef mefdn::unique_ptr<uct_md, memory_domain_deleter>  base;
    
public:
    memory_domain() noexcept = default;
    
    explicit memory_domain(uct_md* const p)
        : base(p)
    { }
    
    memory_domain(const memory_domain&) = delete;
    memory_domain& operator = (const memory_domain&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(memory_domain, base)
};

memory_domain open_memory_domain(const char* md_name, uct_md_config_t*);

memory_domain open_memory_domain(uct_md_resource_desc_t* md_desc);

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

