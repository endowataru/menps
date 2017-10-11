
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/rpc/requester.hpp>
#include <mgbase/unique_ptr.hpp>
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {

mgbase::unique_ptr<rma::requester> make_rma_requester(rpc::requester&, mpi_interface&);

mgbase::unique_ptr<rma::registrator> make_rma_registrator();

} // namespace mpi
} // namespace mgcom

