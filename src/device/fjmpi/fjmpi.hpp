
#pragma once

#include "device/mpi/mpi_base.hpp"
#include "common/starter.hpp"
#include "mpi-ext.h"

namespace mgcom {
namespace fjmpi {

mgbase::unique_ptr<starter> make_starter(int* const argc, char*** const argv);

} // namespace fjmpi
} // namespace mgcom

