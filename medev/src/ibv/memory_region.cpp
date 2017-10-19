
#include <menps/medev/ibv/memory_region.hpp>
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medev {
namespace ibv {

memory_region make_memory_region(
    ibv_pd* const           pd
,   void* const             ptr
,   const mefdn::size_t    size_in_bytes
,   const int               access
) {
    MEFDN_ASSERT(pd != nullptr);
    
    const auto mr =
        ibv_reg_mr(pd, ptr, size_in_bytes, access);
    
    if (mr == nullptr) {
        throw ibv_error("ibv_reg_mr() failed");
    }
    
    MEFDN_LOG_DEBUG(
        "msg:Registered region.\t"
        "ptr:{:x}\tsize_in_bytes:{}\t"
        "lkey:{:x}\trkey:{:x}"
    ,   reinterpret_cast<mefdn::uintptr_t>(ptr)
    ,   size_in_bytes
    ,   mr->lkey
    ,   mr->rkey
    );
    
    return memory_region(mr);
}

void memory_region_deleter::operator () (ibv_mr* const mr) const noexcept
{
    if (mr == nullptr) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_dereg_mr(mr); // ignore error
}

} // namespace ibv
} // namespace medev
} // namespace menps

