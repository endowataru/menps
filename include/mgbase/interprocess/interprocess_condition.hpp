
#pragma one

#include "interprocess_mutex.hpp"
#include <mgbase/threading/unique_lock.hpp>

namespace mgbase {

struct interprocess_condition_error { };

namespace detail {

struct scoped_condattr
{
    scoped_condattr()
    {
        if (pthread_condattr_init(&attr) != 0)
            throw interprocess_condition_error();
    }
    
    ~scoped_condattr()
    {
        if (pthread_condattr_destroy(&attr) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void setpshared()
    {
        if (pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0)
            throw interprocess_condition_error();
    }
    
    pthread_condattr_t attr;
};

} // namespace detail


class interprocess_condition
{
    typedef mgbase::interprocess_mutex  mutex_type;
    
public:
    interprocess_condition()
    {
        detail::scoped_condattr ca;
        ca.setpshared();
        
        if (pthread_cond_init(&cond_, &ca.attr) != 0)
            throw interprocess_condition_error();
    }
    
    ~interprocess_condition()
    {
        if (pthread_cond_destroy(&cond_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void wait(mgbase::unique_lock<mutex_type>& lc)
    {
        if (!lc.owns_lock())
            throw interprocess_condition_error();
        
        mutex_type* const mtx = lc.mutex();
        if (mtx == MGBASE_NULLPTR)
            throw interprocess_condition_error();
        
        pthread_mutex_t* const mtx_ptr = mtx->native_handle();
        
        const bool err = pthread_cond_wait(&cond_, mtx_ptr) != 0;
        
        if (err)
            throw interprocess_condition_error();
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
            throw interprocess_condition_error();
    }
    
    void notify_all()
    {
        if (pthread_cond_broadcast(&cond_) != 0)
            throw interprocess_condition_error();
    }
    
private:
    pthread_cond_t cond_;
};

} // namespace mgbase

