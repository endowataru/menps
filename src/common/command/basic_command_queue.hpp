
#pragma once

#include "basic_command.hpp"
#include <mgbase/scoped_enum.hpp>

namespace mgcom {

template <typename Derived, typename CommandCode>
class basic_command_queue
{
    typedef CommandCode     command_code_type;
    
protected:
    basic_command_queue() MGBASE_EMPTY_DEFINITION
    
public:
    bool try_call(const mgbase::callback_function<void ()>& func)
    {
        const basic_command_parameters::call_parameters call_params = { func };
        
        basic_command_parameters params;
        params.call = call_params;
        
        const bool ret = derived().try_enqueue_basic(
            command_code_type::BASIC_COMMAND_CALL
        ,   params
        );
        
        return ret;
    }

private:
    Derived& derived() MGBASE_NOEXCEPT { return static_cast<Derived&>(*this); }
};

} // namespace mgcom

