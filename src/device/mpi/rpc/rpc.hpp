
#pragma once

#include "common/rpc.hpp"
#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace rpc {

void initialize();

void finalize();

struct message_buffer {
    handler_id_t    id;
    index_t         size;
    int             reply_tag;
    int             reply_size;
    uint8_t         data[MGCOM_RPC_MAX_DATA_SIZE]; // TODO
};

MPI_Comm get_comm();

} // namespace rpc
} // namespace mgcom

