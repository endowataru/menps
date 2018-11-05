
#pragma once

#include "mth.hpp"

#define MEULT_AVOID_MYTH_UNCOND_SETUP

namespace menps {
namespace meult {
namespace backend {
namespace mth {

struct uncond_variable_error { };

class uncond_variable
{
public:
    uncond_variable() /* may throw */
    {
        #ifdef MEULT_AVOID_MYTH_UNCOND_SETUP
        this->u_.th = nullptr;
        
        #else
        myth_uncond_init(&this->u_);
        #endif
    }
    
    ~uncond_variable() {
        #ifndef MEULT_AVOID_MYTH_UNCOND_SETUP
        myth_uncond_destroy(&this->u_); // ignore error
        #endif
    }
    
    uncond_variable(const uncond_variable&) = delete;
    uncond_variable& operator = (const uncond_variable&) = delete;
    
    void wait()
    {
        if (myth_uncond_wait(&this->u_) != 0)
            throw uncond_variable_error();
    }
    
    void notify()
    {
        this->notify_signal();
    }
    
    void notify_signal()
    {
        if (myth_uncond_signal(&this->u_) != 0)
            throw uncond_variable_error();
    }
    void notify_enter()
    {
        #ifdef MEULT_ENABLE_MTH_UNCOND_ENTER
        if (myth_uncond_enter(&this->u_) != 0)
            throw uncond_variable_error();
        #else
        this->notify_signal();
        #endif
    }
    
private:
    myth_uncond_t u_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

