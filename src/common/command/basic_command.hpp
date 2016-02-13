
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/callback_function.hpp>

namespace mgcom {

enum basic_command_code
{
    BASIC_COMMAND_CALL
,   BASIC_COMMAND_END
};

union basic_command_parameters
{
    struct call_parameters {
        mgbase::callback_function<void ()> func;
    }
    call;
};

MGBASE_ALWAYS_INLINE bool execute_on_this_thread(
    const basic_command_code        code
,   const basic_command_parameters& params
) {
    MGBASE_ASSERT(code < BASIC_COMMAND_END);
    
    switch (code)
    {
        case BASIC_COMMAND_CALL: {
            const basic_command_parameters::call_parameters& p = params.call;
            
            // Call the user-defined function.
            p.func();
            
            return true;
        }
        
        case BASIC_COMMAND_END:
        default:
            MGBASE_UNREACHABLE();
            break;
    }
}

} // namespace mgcom

