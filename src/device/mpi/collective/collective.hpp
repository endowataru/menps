
#pragma once

#include <mgcom/collective.hpp>
#include "device/mpi/mpi_interface.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi {
namespace collective {

using namespace mgcom::collective;

mgbase::unique_ptr<requester> make_requester(mpi_interface&);

} // namespace collective
} // namespace mpi
} // namespace mgcom

