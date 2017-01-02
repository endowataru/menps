
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/rma/registrator.hpp>
#include "device/mpi3/mpi3_interface.hpp"
#include <mgbase/unique_ptr.hpp>

#include "rma_window.hpp"

namespace mgcom {
namespace mpi3 {

mgbase::unique_ptr<rma::requester> make_rma_requester(mpi3_interface&, MPI_Win);

mgbase::unique_ptr<rma::registrator> make_rma_registrator(mpi3_interface&, MPI_Win);

} // namespace mpi3
} // namespace mgcom

