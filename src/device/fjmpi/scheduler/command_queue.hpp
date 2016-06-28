
#pragma once

#include "common/command/delegator.hpp"
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include "device/fjmpi/command/command.hpp"

namespace mgcom {
namespace fjmpi {

class command_queue
    : public mgbase::mpsc_circular_buffer<command, command::queue_size> { };

} // namespace fjmpi
} // namespace mgcom

