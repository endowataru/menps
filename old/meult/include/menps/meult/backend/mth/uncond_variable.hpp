
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
    
    void swap(uncond_variable& next_uv)
    {
        const auto p = prof::start();
        
        if (myth_uncond_swap(&this->u_, &next_uv.u_) != 0)
            throw uncond_variable_error();
        
        prof::finish(prof_kind::myth_uncond_swap, p);
    }
    
private:
    template <typename Func>
    static int on_swap(void* const ptr)
    {
        auto& func = *static_cast<Func*>(ptr);
        // TODO: To avoid bugs, copy the function here.
        auto f = func;
        return f();
    }
    
public:
    template <typename Func>
    void swap_with(uncond_variable& next_uv, Func func)
    {
        const auto p = prof::start();
        
        const auto ret =
            myth_uncond_swap_with(
                &this->u_, &next_uv.u_, &on_swap<Func>, &func);
        
        if (ret != 0)
            throw uncond_variable_error();
        
        prof::finish(prof_kind::myth_uncond_swap_with, p);
    }
    
private:
    template <typename Func>
    struct wait_func
    {
        Func func;
        #ifdef MEULT_ENABLE_PROF
        mefdn::cpu_clock_t p;
        #endif
        
        static int on_wait(void* const ptr)
        {
            auto& self = *static_cast<wait_func*>(ptr);
            
            #ifdef MEULT_ENABLE_PROF
            prof::finish(prof_kind::myth_uncond_wait_with_sw, self.p);
            #endif
            
            // TODO: To avoid bugs, copy the function here.
            auto f = self.func;
            return f();
        }
    };
    
public:
    template <typename Func>
    void wait_with(Func func)
    {
        const auto p = prof::start();
        
        wait_func<Func> f{
            mefdn::move(func)
            #ifdef MEULT_ENABLE_PROF
            , p 
            #endif
        };
        
        const auto ret =
            myth_uncond_wait_with(
                &this->u_, &wait_func<Func>::on_wait, &f);
        
        if (ret != 0)
            throw uncond_variable_error();
        
        prof::finish(prof_kind::myth_uncond_wait_with, p);
    }
    
private:
    myth_uncond_t u_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

