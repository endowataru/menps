
#pragma once

#include "rpc_connection_pool.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

void initialize_receiver(rpc_connection_pool& pool);

void finalize_receiver();

void register_handler_to_receiver(const untyped::register_handler_params& params);

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

