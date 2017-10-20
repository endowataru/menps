
#pragma once

#include <menps/mecom/common.hpp>
#include <menps/mefdn/callback.hpp>

namespace menps {
namespace mecom {
namespace rpc {

#define MECOM_RPC_MAX_DATA_SIZE         (64<<10)
#define MECOM_RPC_MAX_NUM_HANDLERS      10000

struct constants
{
    static const index_t max_num_handlers = MECOM_RPC_MAX_NUM_HANDLERS;
};

typedef index_t     handler_id_t;

} // namespace rpc
} // namespace mecom
} // namespace menps

