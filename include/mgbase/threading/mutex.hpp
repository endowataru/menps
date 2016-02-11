
#pragma once

#include <pthread.h>
#include <errno.h>

namespace mgbase {

struct mutex_error { };

class mutex
    : mgbase::noncopyable
{
public:
    typedef pthread_mutex_t*    native_handle_type;
    
    mutex()
    {
        if (pthread_mutex_init(&mtx_, MGBASE_NULLPTR) != 0)
            throw mutex_error();
    }
    
    ~mutex()
    {
        if (pthread_mutex_destroy(&mtx_) != 0) {
            //throw mutex_error(); // Ignore because this is in a destructor
        }
    }
    
    void lock()
    {
        if (pthread_mutex_lock(&mtx_) != 0)
            throw mutex_error();
    }
    
    bool try_lock()
    {
        const int ret = pthread_mutex_lock(&mtx_);
        
        if (ret == 0)
            return true;
        else if (ret == EBUSY)
            return false;
        else
            throw mutex_error();
    }
    
    void unlock()
    {
        if (pthread_mutex_unlock(&mtx_) != 0)
            throw mutex_error();
    }
    
    native_handle_type native_handle() MGBASE_NOEXCEPT {
        return &mtx_;
    }

private:
    pthread_mutex_t mtx_;
};

} // namespace mgbase

