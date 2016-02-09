
#pragma once

#include "common/rpc.hpp"
#include "device/fjmpi/rma/rma.hpp"

namespace mgcom {
namespace rpc {

void initialize();

void finalize();

struct rpc_client_connection;

typedef uint8_t rpc_message_data[MGCOM_RPC_MAX_DATA_SIZE];

typedef mgbase::uint32_t    ticket_t;

struct message_buffer {
    bool                    is_reply;
    handler_id_t            handler_id;
    ticket_t                tickets[rma::constants::max_nic_count];
    index_t                 data_size;
    void*                   return_ptr;
    mgbase::operation       on_complete;
    rpc_message_data        data; // TODO
};

} // namespace rpc
} // namespace mgcom

