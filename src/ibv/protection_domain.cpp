
#include <mgdev/ibv/protection_domain.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/assert.hpp>

namespace mgdev {
namespace ibv {

protection_domain make_protection_domain(ibv_context* const ctx)
{
    MGBASE_ASSERT(ctx != MGBASE_NULLPTR);
    
    const auto pd = ibv_alloc_pd(ctx);
    if (pd == MGBASE_NULLPTR) {
        throw ibv_error("ibv_alloc_pd() failed");
    }
    
    return protection_domain(pd);
}

void protection_domain_deleter::operator () (ibv_pd* const pd) const MGBASE_NOEXCEPT
{
    if (pd == MGBASE_NULLPTR) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_dealloc_pd(pd); // ignore error
}

} // namespace ibv
} // namespace mgdev

