
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/callback_function.hpp>

namespace mgcom {

union basic_command_parameters
{
    struct call_parameters {
        mgbase::callback_function<void ()> func;
    }
    call;
};

MGBASE_ALWAYS_INLINE void execute_call(const basic_command_parameters::call_parameters& params)
{
    // Call the user-defined function.
    params.func();
}

#define MGCOM_BASIC_COMMAND_CODES(x) \
    x(BASIC_COMMAND_CALL)

#define MGCOM_BASIC_COMMAND_EXECUTE_CASES(CASE, RETURN, params) \
    CASE(BASIC_COMMAND_CALL): { \
        mgcom::execute_call(params.call); \
        RETURN(true) \
    }


/*MGBASE_ALWAYS_INLINE bool execute_on_this_thread(
    const basic_command_code        code
,   const basic_command_parameters& params
) {
    switch (code)
    {
        case BASIC_COMMAND_CALL: {
            const basic_command_parameters::call_parameters& p = params.call;
            
            // Call the user-defined function.
            p.func();
            
            return true;
        }
        
        case BASIC_COMMAND_END:
        //#if 0
        default:
        //#endif
            MGBASE_ASSERT(code < BASIC_COMMAND_END);
            MGBASE_UNREACHABLE();
            break;
    }
}*/
} // namespace mgcom

