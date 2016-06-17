
#pragma once

#include "rma.hpp"

namespace mgcom {
namespace mpi {
namespace rma {

void initialize_atomic();

void finalize_atomic();

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params);

MGBASE_WARN_UNUSED_RESULT
bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params);

MGBASE_WARN_UNUSED_RESULT
bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params);

MGBASE_WARN_UNUSED_RESULT
bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params);

} // namespace rma
} // namespace mpi
} // namespace mgcom

