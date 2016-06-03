
#include "rma.impl.hpp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

fjmpi::rma_impl g_impl;

} // unnamed namespace

void initialize_contiguous()
{
    g_impl.initialize();
}
void finalize_contiguous()
{
    g_impl.finalize();
}

namespace untyped {

local_region register_region(
    void* const     local_ptr
,   index_t const   size_in_bytes
) {
    mgbase::uint64_t laddr;
    
    const int memid = g_impl.register_memory(
        local_ptr
    ,   size_in_bytes
    ,   &laddr
    );
    
    return make_local_region(
        make_region_key(
            local_ptr
        ,   static_cast<mgbase::uint64_t>(memid)
        )
    ,   laddr
    );
}

remote_region use_remote_region(
    const process_id_t  proc_id
,   const region_key&   key
) {
    const mgbase::uint64_t raddr
        = g_impl.get_remote_addr(
            static_cast<int>(proc_id)
        ,   static_cast<int>(key.info)
        );
    
    return make_remote_region(key, raddr);
}

void deregister_region(const local_region& region)
{
    g_impl.deregister_memory(
        static_cast<int>(region.key.info)
    );
}

} // namespace untyped

} // namespace rma
} // namespace mgcom

