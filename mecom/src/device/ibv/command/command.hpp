
#pragma once

#include "code.hpp"

namespace menps {
namespace mecom {
namespace ibv {

struct command
{
    // TODO: 
    static const mefdn::size_t params_size = 128 - sizeof(long) - sizeof(void*); // TODO
    
    static const index_t queue_size = 1 << 14; // TODO
    
    command_code code;
    process_id_t proc;
    mefdn::uint8_t arg[params_size];
};

} // namespace ibv
} // namespace mecom
} // namespace menps

