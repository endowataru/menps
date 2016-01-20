
#pragma once

#include <mgcom/rma/untyped.hpp>
#include "common/notifier.hpp"

namespace mgcom {
namespace rma {

namespace untyped {

/**
 * Low-level function of contiguous read.
 */
bool try_remote_read(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   index_t                 size_in_bytes
,   local_notifier          on_complete
);

/**
 * Low-level function of contiguous write.
 */
bool try_remote_write(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   index_t                 size_in_bytes
,   local_notifier          on_complete
);

} // namespace untyped

} // namespace rma
} // namespace mgcom

