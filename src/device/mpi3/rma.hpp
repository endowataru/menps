
#pragma once

#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace rma {

void initialize();
void finalize();

MPI_Win get_win() MGBASE_NOEXCEPT;

} // namespace rma
} // namespace mgcom

