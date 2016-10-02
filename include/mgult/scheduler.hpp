
#pragma once

#include <mgult/generic/common.hpp>
#include "ult_id.hpp"

#include <mgbase/unique_ptr.hpp>

namespace mgult {

class scheduler
{
public:
    virtual ~scheduler() = default;
    
    virtual void loop(loop_func_t) = 0;
    
    virtual ult_id fork(fork_func_t func, void* arg) = 0;
    
    virtual void* join(const ult_id&) = 0;
    
    virtual void detach(const ult_id&) = 0;
    
    virtual void yield() = 0;
    
    MGBASE_NORETURN
    virtual void exit(void* ret) = 0;
};

} // namespace mgult

