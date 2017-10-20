
#pragma once

#include <menps/mecom/common.hpp>
#include <menps/meult/offload/basic_spin_offload_queue.hpp>

namespace menps {
namespace mecom {

template <typename Policy>
class basic_command_queue
    : public meult::basic_spin_offload_queue<Policy>
{ };

} // namespace mecom
} // namespace menps
