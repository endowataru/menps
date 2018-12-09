
#pragma once

#include "requester.hpp"

#ifdef MECOM_USE_SLEEP_RMA
    #include <menps/mecom/command/basic_locked_command_queue.hpp>
#else
    #include <menps/mecom/command/basic_command_queue.hpp>
#endif

namespace menps {
namespace mecom {
namespace rma {

enum class command_code {
    read = 1
,   write
,   atomic_read
,   atomic_write
,   compare_and_swap
,   fetch_and_add
};

struct command
{
    static const mefdn::size_t args_size = 128 - sizeof(command_code); // TODO
    
    command_code        code;
    
    union {
        // TODO: separate "proc" from *_params
        process_id_t    proc;
        
        char arg[args_size];
    }
    arg;
};

struct command_queue_policy
    : mecom::ult::ult_policy
{
    typedef command     command_type;
    
    static const index_t queue_size = 1 << 14; // TODO: magic number
};

#ifdef MECOM_USE_SLEEP_RMA
    typedef basic_locked_command_queue<command_queue_policy>    command_queue;
#else
    typedef basic_command_queue<command_queue_policy>           command_queue;
#endif

} // namespace rma
} // namespace mecom
} // namespace menps

