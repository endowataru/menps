
#pragma once

#include <mgbase/threading/unique_lock.hpp>
#include <mgbase/threading/mutex.hpp>

namespace mgbase {

struct condition_variable_error { };

class condition_variable
    : mgbase::noncopyable
{
    typedef mgbase::mutex   mutex_type;
    
public:
    condition_variable()
    {
        if (pthread_cond_init(&cond_, MGBASE_NULLPTR) != 0)
            throw condition_variable_error();
    }
    
    ~condition_variable()
    {
        if (pthread_cond_destroy(&cond_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void wait(mgbase::unique_lock<mutex_type>& lc)
    {
        if (!lc.owns_lock())
            throw condition_variable_error();
        
        mutex_type* const mtx = lc.mutex();
        if (mtx == MGBASE_NULLPTR)
            throw condition_variable_error();
        
        pthread_mutex_t* const mtx_ptr = mtx->native_handle();
        
        const bool err = pthread_cond_wait(&cond_, mtx_ptr) != 0;
        
        if (err)
            throw condition_variable_error();
    }
    
    template <typename Predicate>
    void wait(mgbase::unique_lock<mutex_type>& lc, Predicate pred)
    {
        while (!pred())
        {
            wait(lc);
        }
    }
    
    void notify_one()
    {
        if (pthread_cond_signal(&cond_) != 0)
            throw condition_variable_error();
    }
    
    void notify_all()
    {
        if (pthread_cond_broadcast(&cond_) != 0)
            throw condition_variable_error();
    }
    
private:
    pthread_cond_t cond_;
};

} // namespace mgbase

