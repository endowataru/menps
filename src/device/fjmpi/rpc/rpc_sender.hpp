
#pragma once

#include "rpc_connection_pool.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

void initialize_sender(rpc_connection_pool& pool);
void finalize_sender();

MGBASE_WARN_UNUSED_RESULT
bool try_call_async(const untyped::call_params& params);

namespace untyped {

bool try_send_reply(
    const int               client_rank
,   const message_buffer&   request_buf
,   void* const             reply_data
,   const int               reply_size
);

} // namespace untyped

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

