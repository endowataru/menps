
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_CXX11_UNIQUE_PTR_SUPPORTED

#include <memory>

namespace mgbase {

using std::unique_ptr;

} // namespace mgbase

#else

namespace mgbase {

template <typename T>
class unique_ptr
{
public: 
    typedef T*  pointer;
    
    unique_ptr() MGBASE_NOEXCEPT
        : ptr_(MGBASE_NULLPTR) { }
    
    explicit unique_ptr(pointer ptr) MGBASE_NOEXCEPT
        : ptr_(ptr) { }
    
    unique_ptr(unique_ptr&& other)
        : ptr_(other.release()) { }
    
    template <typename U>
    unique_ptr(unique_ptr<U>&& other)
        : ptr_(other.release()) { }
    
    ~unique_ptr()
    {
        reset();
    }
    
    unique_ptr& operator = (unique_ptr&& other) MGBASE_NOEXCEPT
    {
        reset(other.release());
        return *this;
    }
    
    template <typename U>
    unique_ptr& operator = (unique_ptr<U>&& other) MGBASE_NOEXCEPT
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

namespace mgbase {

template <typename T, typename... Args>
inline unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(
        new T(std::forward<Args>(args)...)
    );
}

} // namespace mgbase

