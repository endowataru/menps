
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_CXX11_SUPPORTED

#include <memory>

namespace mgbase {

using std::unique_ptr;

} // namespace mgbase

#else

#include <mgbase/move.hpp>

namespace mgbase {

template <typename T>
class unique_ptr
{
public: 
    MGBASE_MOVABLE_BUT_NOT_COPYABLE(unique_ptr)
    
    typedef T*  pointer;
    
    unique_ptr() MGBASE_NOEXCEPT
        : ptr_(MGBASE_NULLPTR) { }
    
    explicit unique_ptr(pointer ptr) MGBASE_NOEXCEPT
        : ptr_(ptr) { }
    
    unique_ptr(MGBASE_RV_REF(unique_ptr) other)
        : ptr_(other.release()) { }
    
    ~unique_ptr()
    {
        reset();
    }
    
    unique_ptr& operator = (MGBASE_RV_REF(unique_ptr) other) MGBASE_NOEXCEPT
    {
        reset(other.release());
        return *this;
    }
    
    void reset(pointer ptr = MGBASE_NULLPTR)
    {
        const pointer old = ptr_;
        if (old != MGBASE_NULLPTR) {
            delete old;
        }
        
        ptr_ = ptr;
    }
    
    pointer release()
    {
        const pointer ptr = ptr_;
        ptr_ = MGBASE_NULLPTR;
        return ptr;
    }
    
    pointer get() const MGBASE_NOEXCEPT {
        return ptr_;
    }
    
    T& operator * () const MGBASE_NOEXCEPT {
        return *get();
    }
    
    pointer operator -> () const MGBASE_NOEXCEPT {
        return get();
    }
    
private:
    pointer ptr_;
};

} // namespace mgbase

#endif

