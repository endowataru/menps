
#pragma once

#include <mgcom/rma.hpp>
#include <mgbase/unique_ptr.hpp>
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {
namespace rma {

using namespace mgcom::rma;

namespace untyped = mgcom::rma::untyped;

mgbase::unique_ptr<requester> make_requester(mpi_interface&);

mgbase::unique_ptr<registrator> make_registrator();

} // namespace rma
} // namespace mpi
} // namespace mgcom

