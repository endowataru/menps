
#pragma once

#include <mgcom/rpc/requester.hpp>
#include <mgcom/collective/requester.hpp>
#include "device/fjmpi/fjmpi_interface.hpp"
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace fjmpi {

mgbase::unique_ptr<rpc::requester> make_rpc_requester(fjmpi_interface& fi, mpi::mpi_interface& mi);

} // namespace fjmpi
} // namespace mgcom

