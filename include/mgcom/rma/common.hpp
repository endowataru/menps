
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/rma/common.h>

namespace mgcom {
namespace rma {

/**
 * Default integer type for atomic operations.
 */
typedef mgcom_rma_atomic_default_t              atomic_default_t;

// TODO: Hide polling function
void poll();

} // namespace rma
} // namespace mgcom

