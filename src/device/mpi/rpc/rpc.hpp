
#pragma once

#include "common/rpc.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {
namespace rpc {

using namespace mgcom::rpc;

namespace untyped = mgcom::rpc::untyped;

mgbase::unique_ptr<requester> make_requester(mpi_interface&, endpoint&);

struct message_buffer {
    handler_id_t    id;
    index_t         size;
    int             reply_tag;
    int             reply_size;
    uint8_t         data[MGCOM_RPC_MAX_DATA_SIZE]; // TODO
};

} // namespace rpc
} // namespace mpi
} // namespace mgcom

