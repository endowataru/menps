
#pragma once

#include "command_queue.hpp"
#include "mpi3_delegator.hpp"
#include "common/command/queue_delegator.hpp"
#include <mgbase/implicit_cast.hpp>
#include "device/mpi3/rma/rma_window.hpp"

namespace mgcom {
namespace mpi3 {

class command_producer
    : public virtual command_queue
{
public:
    command_producer(endpoint& ep, mpi::mpi_completer_base& comp, rma_window& win)
        : del_{*this}
        , mpi_{ep, del_, comp, win.get()} {}
    
    mpi3_interface& get_mpi_interface() MGBASE_NOEXCEPT {
        return mpi_;
    }
    
private:
    queue_delegator<command_queue> del_;
    mpi3_delegator mpi_;
};

} // namespace mpi1
} // namespace mgcom

