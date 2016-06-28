
#pragma once

#include "rma.hpp"
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {
namespace rma {

void initialize_contiguous(mpi_interface&);

void finalize_contiguous();

bool try_read_async(const untyped::read_params& params);

bool try_write_async(const untyped::write_params& params);

} // namespace rma
} // namespace mpi
} // namespace mgcom

