
#pragma once

#include "rpc.hpp"
#include "common/rpc/rpc_basic_connection_pool.hpp"
#include "device/fjmpi/fjmpi_interface.hpp"
#include "device/mpi/mpi_interface.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {
namespace rpc {

void initialize(fjmpi_interface&, mpi::mpi_interface&, rma::allocator&, rma::registrator&);

void finalize();

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
} // namespace mecom
} // namespace menps

