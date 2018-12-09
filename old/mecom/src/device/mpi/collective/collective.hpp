
#pragma once

#include <menps/mecom/collective.hpp>
#include "device/mpi/mpi_interface.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace mpi {
namespace collective {

using namespace mecom::collective;

mefdn::unique_ptr<requester> make_requester(mpi_interface&);

} // namespace collective
} // namespace mpi
} // namespace mecom
} // namespace menps

