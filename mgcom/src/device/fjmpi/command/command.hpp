
#pragma once

#include "code.hpp"

namespace mgcom {
namespace fjmpi {

#define DEFINE_ENUM(x)  x

enum class command_code
{
    call = 1
,   MGCOM_FJMPI_COMMAND_CODES(DEFINE_ENUM)
};

#undef DEFINE_ENUM

struct command
{
    // TODO: 
    static const mgbase::size_t params_size = 128 - sizeof(long) - sizeof(void*); // TODO
    
    static const index_t queue_size = 4; // TODO
    
    command_code code;
    bool (*func)(const void*);
    mgbase::uint8_t arg[params_size];
    
    inline void set_delegated(const delegator::execute_params& params)
    {
        params.setter(arg);
        
        func = params.delegated;
        
        code = command_code::call;
    }
};

} // namespace fjmpi
} // namespace mgcom

