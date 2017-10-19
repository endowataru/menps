
#include <menps/medev/ibv/protection_domain.hpp>
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medev {
namespace ibv {

protection_domain make_protection_domain(ibv_context* const ctx)
{
    MEFDN_ASSERT(ctx != nullptr);
    
    const auto pd = ibv_alloc_pd(ctx);
    if (pd == nullptr) {
        throw ibv_error("ibv_alloc_pd() failed");
    }
    
    return protection_domain(pd);
}

void protection_domain_deleter::operator () (ibv_pd* const pd) const noexcept
{
    if (pd == nullptr) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_dealloc_pd(pd); // ignore error
}

} // namespace ibv
} // namespace medev
} // namespace menps

