
#pragma once

#include "mth.hpp"

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
        myth_uncond_init(&this->u_);
    }
    
    ~uncond_variable() {
        myth_uncond_destroy(&this->u_); // ignore error
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
        if (myth_uncond_signal(&this->u_) != 0)
            throw uncond_variable_error();  
    }
    
private:
    myth_uncond_t u_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

