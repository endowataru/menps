
#include "contiguous.impl.hpp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

untyped::emulated_contiguous g_impl;

} // unnamed namespace

void initialize_contiguous()
{
    untyped::emulated_contiguous::initialize<g_impl>();
}

void finalize_contiguous()
{
    g_impl.finalize();
}

namespace untyped {

bool try_remote_read_async(
    process_id_t                    proc
,   const remote_address&           remote_addr
,   const local_address&            local_addr
,   index_t                         size_in_bytes
,   const mgbase::operation&        on_complete
) {
    return emulated_contiguous::try_read<g_impl>(
        proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   on_complete
    );
}

bool try_remote_write_async(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return emulated_contiguous::try_write<g_impl>(
        proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   on_complete
    );
}

} // namespace untyped

} // namespace rma
} // namespace mgcom

