
#pragma once

#include <menps/mecom/rpc/requester.hpp>
#include <menps/mecom/collective/requester.hpp>
#include "device/fjmpi/fjmpi_interface.hpp"
#include "device/mpi/mpi_interface.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

mefdn::unique_ptr<rpc::requester> make_rpc_requester(fjmpi_interface&, mpi::mpi_interface&, rma::allocator&, rma::registrator&);

} // namespace fjmpi
} // namespace mecom
} // namespace menps

