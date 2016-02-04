
#pragma once

#include <mgcom.hpp>
#include "common/rma/rma.hpp"

namespace mgcom {
namespace rma {

void initialize_contiguous();
void finalize_contiguous();

namespace untyped {

bool try_remote_read_async_extra(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   int                         flags
,   const mgbase::operation&    on_complete
);

bool try_remote_write_async_extra(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   int                         flags
,   const mgbase::operation&    on_complete
);

} // namespace untyped

} // namespace rma
} // namespace mgcom

