
#pragma once

#include "rpc.hpp"
#include "common/rpc/rpc_basic_connection_pool.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

typedef rpc_basic_connection_pool<
    message_buffer
,   ticket_t
,   16
,   int
,   4
>
rpc_connection_pool;

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

