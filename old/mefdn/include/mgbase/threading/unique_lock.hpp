
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

template <typename Mutex>
class unique_lock
{
public:
    typedef Mutex   mutex_type;
    
    unique_lock()
        : mtx_(MGBASE_NULLPTR), locked_(false)
    { }
    
    explicit unique_lock(mutex_type& mtx) MGBASE_NOEXCEPT
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
    
    unique_lock(const unique_lock&) = delete;
    unique_lock& operator = (const unique_lock&) = delete;
    
    unique_lock(unique_lock&& other)
        : mtx_(MGBASE_NULLPTR), locked_(false)
    {
        *this = mgbase::move(other);
    }
    unique_lock& operator = (unique_lock&& other) MGBASE_NOEXCEPT
    {
        unlock_if_locked();
        
        this->mtx_ = other.mtx_;
        this->locked_ = other.locked_;
        
        other.mtx_ = MGBASE_NULLPTR;
        other.locked_ = false;
        
        return *this;
    }
    
    ~unique_lock()
    {
        unlock_if_locked();
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
        this->mtx_ = MGBASE_NULLPTR;
        this->locked_ = false;
        return mtx;
    }
    
    mutex_type* mutex() const MGBASE_NOEXCEPT {
        return mtx_;
    }
    
    bool owns_lock() const MGBASE_NOEXCEPT {
        return locked_;
    }
    
private:
    void unlock_if_locked()
    {
        if (this->owns_lock())
            this->unlock();
    }
    
    Mutex*  mtx_;
    bool    locked_;
};

} // namespace mgbase

#endif // MGBASE_CXX11_SUPPORTED

