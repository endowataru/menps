
#pragma once

#include "code.hpp"

namespace mgcom {
namespace ibv {

struct command
{
    // TODO: 
    static const mgbase::size_t params_size = 128 - sizeof(long) - sizeof(void*); // TODO
    
    static const index_t queue_size = 1 << 14; // TODO
    
    command_code code;
    process_id_t proc;
    mgbase::uint8_t arg[params_size];
};

} // namespace ibv
} // namespace mgcom

