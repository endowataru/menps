
#pragma once

#include "common/command/delegator.hpp"
#include <menps/mefdn/nonblocking/mpsc_bounded_queue.hpp>
#include "device/fjmpi/command/command.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class command_queue
    : public mefdn::static_mpsc_bounded_queue<command, command::queue_size> { };

} // namespace fjmpi
} // namespace mecom
} // namespace menps

