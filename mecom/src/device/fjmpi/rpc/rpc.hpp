
#pragma once

#include "common/rpc.hpp"
#include "device/fjmpi/rma/rma.hpp"
#include "device/fjmpi/fjmpi.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {
namespace rpc {

using namespace mecom::rpc;

namespace untyped {

using namespace mecom::rpc::untyped;

} // namespace untyped

struct rpc_client_connection;

typedef uint8_t rpc_message_data[MECOM_RPC_MAX_DATA_SIZE];

typedef mefdn::uint32_t    ticket_t;

struct message_buffer {
    bool                    is_reply;
    handler_id_t            handler_id;
    ticket_t                tickets[fjmpi::constants::max_nic_count];
    index_t                 data_size;
    void*                   return_ptr;
    mefdn::callback<void ()>   on_complete;
    rpc_message_data        data; // TODO
};

} // namespace rpc
} // namespace fjmpi
} // namespace mecom
} // namespace menps

