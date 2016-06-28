
#pragma once

#include <mgbase/callback_function.hpp>
#include <mgbase/ult.hpp>
#include <mgcom/common.hpp>

#include <mgbase/logger.hpp>

namespace mgcom {

class delegator
{
protected:
    delegator() /*MGBASE_NOEXCEPT*/ = default;

public:
    virtual ~delegator() MGBASE_EMPTY_DEFINITION
    
    delegator(const delegator&) = delete;
    delegator& operator = (const delegator&) = delete;
    
    struct execute_params
    {
        mgbase::callback_function<void (void*)> setter;
        bool (*delegated) (const void*);
    };
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_execute(const execute_params&) = 0;
    
    virtual mgbase::size_t get_buffer_size() = 0;
};

} // namespace mgcom

