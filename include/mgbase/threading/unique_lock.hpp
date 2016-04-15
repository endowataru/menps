
#pragma once

#include <mgbase/threading/lock_t.hpp>

#ifdef MGBASE_CXX11_SUPPORTED

#include <mutex>

namespace mgbase {

using std::unique_lock;

} // namespace mgbase

#else // MGBASE_CXX11_SUPPORTED

namespace mgbase {

class condition_variable; // for friend declaration

struct unique_lock_error { };

// Because it is impossible to implement the full features of unique_lock in C++03,
// it is defined as a restricted version.

template <typename Mutex>
class unique_lock
{
public:
    typedef Mutex   mutex_type;
    
    explicit unique_lock(mutex_type& mtx)
        : mtx_(&mtx), locked_(true)
    {
        mtx.lock();
    }
    
    unique_lock(mutex_type& mtx, defer_lock_t) MGBASE_NOEXCEPT
        : mtx_(&mtx), locked_(false) { }
    
    unique_lock(mutex_type& mtx, adopt_lock_t) MGBASE_NOEXCEPT
        : mtx_(&mtx), locked_(true) { }
    
    unique_lock(mutex_type& mtx, try_to_lock_t)
        : mtx_(&mtx)
        , locked_(mtx.try_lock()) { }
    
    ~unique_lock()
    {
        if ((mtx_ != MGBASE_NULLPTR) && locked_)
            mtx_->unlock();
    }
    
    void lock()
    {
        if ((mtx_ == MGBASE_NULLPTR) || locked_)
            throw unique_lock_error();
        
        mtx_->lock();
        locked_ = true;
    }
    
    bool try_lock()
    {
        if ((mtx_ == MGBASE_NULLPTR) || locked_)
            throw unique_lock_error();
        
        locked_ = mtx_->try_lock();
        return locked_;
    }
    
    void unlock()
    {
        if ((mtx_ == MGBASE_NULLPTR) || !locked_)
            throw unique_lock_error();
        
        mtx_->unlock();
        locked_ = false;
    }
    
    mutex_type* release() MGBASE_NOEXCEPT
    {
        mutex_type* const mtx = mtx_;
        mtx_ = MGBASE_NULLPTR;
        return mtx;
    }
    
    mutex_type* mutex() const MGBASE_NOEXCEPT
    {
        return mtx_;
    }
    
    bool owns_lock() const MGBASE_NOEXCEPT
    {
        return locked_;
    }
    
private:
    Mutex* mtx_;
    bool locked_;
};

} // namespace mgbase

#endif // MGBASE_CXX11_SUPPORTED

