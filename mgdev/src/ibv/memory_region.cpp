
#include <mgdev/ibv/memory_region.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/logger.hpp>

namespace mgdev {
namespace ibv {

memory_region make_memory_region(
    ibv_pd* const           pd
,   void* const             ptr
,   const mgbase::size_t    size_in_bytes
,   const int               access
) {
    MGBASE_ASSERT(pd != MGBASE_NULLPTR);
    
    const auto mr =
        ibv_reg_mr(pd, ptr, size_in_bytes, access);
    
    if (mr == MGBASE_NULLPTR) {
        throw ibv_error("ibv_reg_mr() failed");
    }
    
    MGBASE_LOG_DEBUG(
        "msg:Registered region.\t"
        "ptr:{:x}\tsize_in_bytes:{}\t"
        "lkey:{:x}\trkey:{:x}"
    ,   reinterpret_cast<mgbase::uintptr_t>(ptr)
    ,   size_in_bytes
    ,   mr->lkey
    ,   mr->rkey
    );
    
    return memory_region(mr);
}

void memory_region_deleter::operator () (ibv_mr* const mr) const MGBASE_NOEXCEPT
{
    if (mr == MGBASE_NULLPTR) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_dereg_mr(mr); // ignore error
}

} // namespace ibv
} // namespace mgdev

