
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/rma/registrator.hpp>
#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace mpi3 {
namespace rma {

using namespace mgcom::rma;

namespace untyped = mgcom::rma::untyped;

mgbase::unique_ptr<requester> make_requester();

mgbase::unique_ptr<registrator> make_registrator();

MPI_Win get_win() MGBASE_NOEXCEPT;

} // namespace rma
} // namespace mpi3
} // namespace mgcom

