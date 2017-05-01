
#pragma once

#include "requester.hpp"
#include <mgcom/command/basic_locked_command_queue.hpp>
#include <mgcom/command/basic_command_queue.hpp>

namespace mgcom {
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
    static const mgbase::size_t args_size = 128 - sizeof(long) - sizeof(void*); // TODO
    
    command_code        code;
    
    union {
        // TODO: separate "proc" from *_params
        process_id_t    proc;
        
        char arg[args_size];
    }
    arg;
};

struct command_queue_policy
{
    typedef command     command_type;
    
    static const index_t queue_size = 1 << 14; // TODO: magic number
};

//typedef basic_locked_command_queue<command_queue_policy>  command_queue;
typedef basic_command_queue<command_queue_policy>           command_queue;

} // namespace rma
} // namespace mgcom

