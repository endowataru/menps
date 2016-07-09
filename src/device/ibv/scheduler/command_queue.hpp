
#pragma once

#include "device/ibv/command/command.hpp"
#include <mgbase/lockfree/mpsc_blocking_bounded_queue.hpp>
//#include <mgbase/lockfree/mpsc_circular_buffer.hpp>

namespace mgcom {
namespace ibv {

class command_queue
    : public mgbase::mpsc_blocking_bounded_queue<command, command::queue_size> { };
    //: public mgbase::mpsc_circular_buffer<command, command::queue_size> { };

} // namespace ibv
} // namespace mgcom

