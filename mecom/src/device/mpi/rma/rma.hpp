
#pragma once

#include <menps/mecom/rma/requester.hpp>
#include <menps/mecom/rpc/requester.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include "device/mpi/mpi_interface.hpp"

namespace menps {
namespace mecom {
namespace mpi {

mefdn::unique_ptr<rma::requester> make_rma_requester(rpc::requester&, mpi_interface&);

mefdn::unique_ptr<rma::registrator> make_rma_registrator();

} // namespace mpi
} // namespace mecom
} // namespace menps

