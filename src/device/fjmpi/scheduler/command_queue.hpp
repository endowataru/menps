
#pragma once

#include "common/command/delegator.hpp"
#include <mgbase/nonblocking/mpsc_bounded_queue.hpp>
#include "device/fjmpi/command/command.hpp"

namespace mgcom {
namespace fjmpi {

class command_queue
    : public mgbase::static_mpsc_bounded_queue<command, command::queue_size> { };

} // namespace fjmpi
} // namespace mgcom

