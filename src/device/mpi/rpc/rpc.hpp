
#pragma once

#include "common/rpc.hpp"
#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

using namespace mgcom::rpc;

namespace untyped = mgcom::rpc::untyped;

mgbase::unique_ptr<requester> make_requester();

struct message_buffer {
    handler_id_t    id;
    index_t         size;
    int             reply_tag;
    int             reply_size;
    uint8_t         data[MGCOM_RPC_MAX_DATA_SIZE]; // TODO
};

MPI_Comm get_comm();


bool try_call_async(const untyped::call_params& params);

void register_handler(const untyped::register_handler_params& params);

} // namespace rpc
} // namespace mpi
} // namespace mgcom

