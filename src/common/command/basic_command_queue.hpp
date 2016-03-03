
#pragma once

#include "basic_command.hpp"

namespace mgcom {

class basic_command_queue
{
protected:
    basic_command_queue() MGBASE_EMPTY_DEFINITION
    
public:
    virtual ~basic_command_queue() MGBASE_EMPTY_DEFINITION

protected:
    virtual bool try_enqueue_basic(
        basic_command_code              code
    ,   const basic_command_parameters& params
    ) = 0;
    
public:
    bool try_call(const mgbase::callback_function<void ()>& func)
    {
        const basic_command_parameters::call_parameters call_params = { func };
        
        basic_command_parameters params;
        params.call = call_params;
        
        const bool ret = try_enqueue_basic(BASIC_COMMAND_CALL, params);
        
        return ret;
    }
};

} // namespace mgcom

