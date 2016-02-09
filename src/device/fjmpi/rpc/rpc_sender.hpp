
#pragma once

#include "rpc_connection_pool.hpp"

namespace mgcom {
namespace rpc {

void initialize_sender(rpc_connection_pool& pool);
void finalize_sender();

namespace untyped {

bool try_send_reply(
    const int               client_rank
,   const message_buffer&   request_buf
,   void* const             reply_data
,   const int               reply_size
);

} // namespace untyped

} // namespace rpc
} // namespace mgcom

