
#include "common/rma/rma.hpp"
#include "rma.hpp"
#include "atomic.hpp"
#include "contiguous.hpp"

#include <mgbase/logging/logger.hpp>

#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace rma {

void initialize() {
    initialize_contiguous();
    initialize_atomic();
    initialize_allocator();
}

void finalize()
{
    finalize_allocator();
    finalize_atomic();
    finalize_contiguous();
}

namespace untyped {

local_region register_region(
    void* const     ptr
,   const index_t   size_in_bytes MGBASE_UNUSED
) {
    // do nothing
    
    MGBASE_LOG_DEBUG(
        "msg:Registered region (but doing nothing.)\tptr:{:x}\tsize_in_bytes:{}"
    ,   reinterpret_cast<mgbase::uint64_t>(ptr)
    ,   size_in_bytes
    );
    
    return make_local_region(make_region_key(ptr, 0 /*unused*/), 0);
}

remote_region use_remote_region(
    process_id_t      //proc_id
,   const region_key& key
) {
    MGBASE_LOG_DEBUG(
        "msg:Use remote region. (but doing nothing.)"
    );
    
    return make_remote_region(key, 0 /* unused */);
}

void deregister_region(const local_region& /*region*/)
{
    // do nothing
}

} // namespace untyped

} // namespace rma
} // namespace mgcom

