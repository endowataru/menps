
#pragma once

#include "mutex.hpp"
#include <menps/mefdn/mutex.hpp>

namespace menps {
namespace meult {
namespace backend {
namespace mth {

struct condition_variable_error { };

class condition_variable
{
    typedef mutex   mutex_type;
    
public:
    condition_variable()
    {
        // TODO: On old GCC (4.4), we cannot use {} on member initialization (?).
        myth_cond_t cond = MYTH_COND_INITIALIZER;
        cond_ = cond;
    }
    
    condition_variable(const condition_variable&) = delete;
    condition_variable& operator = (const condition_variable&) = delete;
    
    ~condition_variable()
    {
        if (myth_cond_destroy(&cond_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void wait(mefdn::unique_lock<mutex_type>& lk)
    {
        if (!lk.owns_lock())
            throw condition_variable_error();
        
        const auto mtx = lk.mutex();
        if (mtx == nullptr)
            throw condition_variable_error();
        
        const auto mtx_ptr = mtx->native_handle();
        
        const auto ret = myth_cond_wait(&cond_, mtx_ptr);
        if (ret != 0)
            throw condition_variable_error();
    }
    
    template <typename Predicate>
    void wait(mefdn::unique_lock<mutex_type>& lc, Predicate pred)
    {
        while (!pred())
        {
            wait(lc);
        }
    }
    
    void notify_one()
    {
        if (myth_cond_signal(&cond_) != 0)
            throw condition_variable_error();
    }
    
    void notify_all()
    {
        if (myth_cond_broadcast(&cond_) != 0)
            throw condition_variable_error();
    }
    
private:
    myth_cond_t cond_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

