
#pragma once

#include "common/rpc.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_interface.hpp"

namespace menps {
namespace mecom {
namespace mpi {

mefdn::unique_ptr<rpc::requester> make_rpc_requester(mpi_interface&, endpoint&);

struct rpc_message_buffer {
    rpc::handler_id_t   id;
    index_t             size;
    int                 reply_tag;
    int                 reply_size;
    uint8_t             data[MECOM_RPC_MAX_DATA_SIZE]; // TODO
};

} // namespace mpi
} // namespace mecom
} // namespace menps

