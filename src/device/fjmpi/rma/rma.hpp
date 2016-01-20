
#pragma once

#include <mgcom.hpp>
#include "common/rma/rma.hpp"
#include "common/notifier.hpp"

namespace mgcom {
namespace rma {

void initialize_contiguous();
void finalize_contiguous();

namespace untyped {

bool try_remote_read_extra(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   index_t                 size_in_bytes
,   local_notifier          on_complete
,   int                     flags
);

bool try_remote_write_extra(
    process_id_t            proc
,   const remote_address&   remote_addr
,   const local_address&    local_addr
,   index_t                 size_in_bytes
,   local_notifier          on_complete
,   int                     flags
);

} // namespace untyped

} // namespace rma
} // namespace mgcom

