
#pragma once

#include <mgcom/common.hpp>
#include <mgult/offload/basic_spin_offload_queue.hpp>

namespace mgcom {

template <typename Policy>
class basic_command_queue
    : public mgult::basic_spin_offload_queue<Policy>
{ };

} // namespace mgcom
