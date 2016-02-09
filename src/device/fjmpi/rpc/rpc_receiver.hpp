
#pragma once

#include "rpc_connection_pool.hpp"

namespace mgcom {
namespace rpc {

void initialize_receiver(rpc_connection_pool& pool);

void finalize_receiver();

} // namespace rpc
} // namespace mgcom

