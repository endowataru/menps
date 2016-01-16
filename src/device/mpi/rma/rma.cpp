
#include "common/rma/rma.hpp"

namespace mgcom {
namespace rma {

void initialize_contiguous();
void initialize_atomic();

void initialize() {
    initialize_contiguous();
    initialize_atomic();
}

void finalize()
{
    // do nothing
}

namespace untyped {

local_region register_region(
    void*   local_pointer
,   index_t //size_in_bytes
) {
    // do nothing
    
    return make_local_region(make_region_key(local_pointer, 0 /*unused*/), 0);
}

remote_region use_remote_region(
    process_id_t      //proc_id
,   const region_key& key
) {
    return make_remote_region(key, 0 /* unused */);
}

void deregister_region(const local_region& /*region*/)
{
    // do nothing
}

} // namespace untyped

} // namespace rma
} // namespace mgcom

