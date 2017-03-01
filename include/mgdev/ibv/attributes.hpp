
#pragma once

#include <mgdev/ibv/verbs.hpp>

namespace mgdev {
namespace ibv {

ibv_device_attr query_device(ibv_context*);

ibv_port_attr query_port(ibv_context*, port_num_t);

} // namespace ibv
} // namespace mgdev

