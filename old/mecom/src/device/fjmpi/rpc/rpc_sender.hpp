
#pragma once

#include "rpc_connection_pool.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {
namespace rpc {

void initialize_sender(fjmpi_interface& fi, rpc_connection_pool& pool);
void finalize_sender();

MEFDN_NODISCARD
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
} // namespace mecom
} // namespace menps

