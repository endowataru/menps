
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/callback.hpp>

namespace mgcom {
namespace rpc {

#define MGCOM_RPC_MAX_DATA_SIZE         1024
#define MGCOM_RPC_MAX_NUM_HANDLERS      10000

struct constants
{
    static const index_t max_num_handlers = MGCOM_RPC_MAX_NUM_HANDLERS;
};

typedef index_t     handler_id_t;

} // namespace rpc
} // namespace mgcom

