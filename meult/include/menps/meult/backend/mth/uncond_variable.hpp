
#pragma once

#include "mth.hpp"
#include <menps/meult/prof.hpp>

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
        const auto p = prof::start();
        
        if (myth_uncond_wait(&this->u_) != 0)
            throw uncond_variable_error();
        
        prof::finish(prof_kind::myth_uncond_wait, p);
    }
    
    void notify()
    {
        this->notify_signal();
    }
    
    void notify_signal()
    {
        const auto p = prof::start();
        
        if (myth_uncond_signal(&this->u_) != 0)
            throw uncond_variable_error();
        
        prof::finish(prof_kind::myth_uncond_signal, p);
    }
    void notify_enter()
    {
        const auto p = prof::start();
        
        #ifdef MEULT_ENABLE_MTH_UNCOND_ENTER
        if (myth_uncond_enter(&this->u_) != 0)
            throw uncond_variable_error();
        #else
        this->notify_signal();
        #endif
        
        prof::finish(prof_kind::myth_uncond_enter, p);
    }
    
private:
    myth_uncond_t u_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

