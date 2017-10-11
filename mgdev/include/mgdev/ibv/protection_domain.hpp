
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ibv {

struct protection_domain_deleter
{
    void operator () (ibv_pd*) const MGBASE_NOEXCEPT;
};

class protection_domain
    : public mgbase::unique_ptr<ibv_pd, protection_domain_deleter>
{
    typedef mgbase::unique_ptr<ibv_pd, protection_domain_deleter>   base;
    
public:
    protection_domain() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit protection_domain(ibv_pd* const pd)
        : base(pd)
    { }
    
    ~protection_domain() /*noexcept*/ = default;
    
    protection_domain(const protection_domain&) = delete;
    protection_domain& operator = (const protection_domain&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(protection_domain, base)
};

protection_domain make_protection_domain(ibv_context*);

} // namespace ibv
} // namespace mgdev

