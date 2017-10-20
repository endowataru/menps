
#pragma once

#include "common/command/delegator.hpp"
#include <menps/mefdn/nonblocking/mpsc_bounded_queue.hpp>

namespace menps {
namespace mecom {
namespace mpi1 {

struct command
{
    // TODO: 
    static const mefdn::size_t params_size = 128 - sizeof(void*); // TODO
    
    static const index_t queue_size = 128; // TODO
    
    bool (*func)(const void*);
    mefdn::uint8_t arg[params_size];
    
    inline void set_delegated(const delegator::execute_params& params)
    {
        params.setter(arg);
        
        func = params.delegated;
    }
};

class command_queue
    : public mefdn::static_mpsc_bounded_queue<command, command::queue_size>
{
};

} // namespace mpi1
} // namespace mecom
} // namespace menps

