
#pragma once

#include <pthread.h>
#include <errno.h>

namespace mgbase {

// See also: Boost.InterProcess

struct interprocess_mutex_error { };

namespace detail {

struct scoped_mutexattr
{
    scoped_mutexattr()
    {
        if (pthread_mutexattr_init(&attr) != 0)
            throw interprocess_mutex_error();
    }
    
    ~scoped_mutexattr()
    {
        if (pthread_mutexattr_destroy(&attr) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void setpshared()
    {
        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0)
            throw interprocess_mutex_error();
    }
    
    pthread_mutexattr_t attr;
};

} // namespace detail


class interprocess_mutex
    : mgbase::noncopyable
{
public:
    typedef pthread_mutex_t*    native_handle_type;
    
    interprocess_mutex()
    {
        detail::scoped_mutexattr ma;
        ma.setpshared();
        
        if (pthread_mutex_init(&mtx_, &ma.attr) != 0)
            throw interprocess_mutex_error();
    }
    
    ~interprocess_mutex()
    {
        if (pthread_mutex_destroy(&mtx_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void lock()
    {
        if (pthread_mutex_lock(&mtx_) != 0)
            throw interprocess_mutex_error();
    }
    
    bool try_lock()
    {
        const int ret = pthread_mutex_lock(&mtx_);
        
        if (ret == 0)
            return true;
        else if (ret == EBUSY)
            return false;
        else
            throw interprocess_mutex_error();
    }
    
    void unlock()
    {
        if (pthread_mutex_unlock(&mtx_) != 0)
            throw interprocess_mutex_error();
    }
    
    native_handle_type native_handle() MGBASE_NOEXCEPT {
        return &mtx_;
    }

private:
    pthread_mutex_t mtx_;
};

} // namespace mgbase

