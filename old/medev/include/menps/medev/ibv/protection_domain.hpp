
#pragma once

#include <menps/medev/ibv/verbs.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ibv {

struct protection_domain_deleter
{
    void operator () (ibv_pd*) const noexcept;
};

class protection_domain
    : public mefdn::unique_ptr<ibv_pd, protection_domain_deleter>
{
    typedef mefdn::unique_ptr<ibv_pd, protection_domain_deleter>   base;
    
public:
    protection_domain() noexcept = default;
    
    explicit protection_domain(ibv_pd* const pd)
        : base(pd)
    { }
    
    ~protection_domain() /*noexcept*/ = default;
    
    protection_domain(const protection_domain&) = delete;
    protection_domain& operator = (const protection_domain&) = delete;
    
    protection_domain(protection_domain&&) noexcept = default;
    protection_domain& operator = (protection_domain&&) noexcept = default;
};

protection_domain make_protection_domain(ibv_context*);

} // namespace ibv
} // namespace medev
} // namespace menps

