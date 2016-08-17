
#pragma once

#include "device/ibv/command/command.hpp"
#include <mgbase/nonblocking/mpsc_bounded_queue.hpp>

namespace mgcom {
namespace ibv {

class command_queue
    : public mgbase::static_mpsc_bounded_queue<command, command::queue_size> { };

} // namespace ibv
} // namespace mgcom

