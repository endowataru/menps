
#pragma once

#include "ult_id.hpp"

#include <mgbase/unique_ptr.hpp>

namespace mgult {

typedef void* (*fork_func_t)(void*);

typedef void (*loop_func_t)();

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

typedef mgbase::unique_ptr<scheduler>   scheduler_ptr;

scheduler_ptr make_scheduler();

} // namespace mgult

