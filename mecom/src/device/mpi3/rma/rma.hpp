
#pragma once

#include <menps/mecom/rma/requester.hpp>
#include <menps/mecom/rma/registrator.hpp>
#include "device/mpi3/mpi3_interface.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

#include "rma_window.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

mefdn::unique_ptr<rma::requester> make_rma_requester(mpi3_interface&, MPI_Win);

mefdn::unique_ptr<rma::registrator> make_rma_registrator(mpi3_interface&, MPI_Win);

} // namespace mpi3
} // namespace mecom
} // namespace menps

