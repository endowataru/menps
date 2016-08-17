
#pragma once

#include "common/command/delegator.hpp"
#include <mgbase/nonblocking/mpsc_bounded_queue.hpp>

namespace mgcom {
namespace mpi1 {

struct command
{
    // TODO: 
    static const mgbase::size_t params_size = 128 - sizeof(void*); // TODO
    
    static const index_t queue_size = 4; // TODO
    
    bool (*func)(const void*);
    mgbase::uint8_t arg[params_size];
    
    inline void set_delegated(const delegator::execute_params& params)
    {
        params.setter(arg);
        
        func = params.delegated;
    }
};

class command_queue
    : public mgbase::static_mpsc_bounded_queue<command, command::queue_size>
{
};

} // namespace mpi1
} // namespace mgcom

