
#pragma once

#include "command_queue.hpp"
#include "device/mpi/command/mpi_delegator.hpp"
#include "common/command/queue_delegator.hpp"

namespace mgcom {
namespace mpi1 {

class command_producer
    : public virtual command_queue
{
public:
    command_producer(mpi::mpi_completer_base& comp)
        : del_{*this}
        , mpi_{del_, comp} {}
    
    mpi::mpi_interface& get_mpi_interface() MGBASE_NOEXCEPT {
        return mpi_;
    }
    
private:
    queue_delegator<command_queue> del_;
    mpi::mpi_delegator mpi_;
};

} // namespace mpi1
} // namespace mgcom

