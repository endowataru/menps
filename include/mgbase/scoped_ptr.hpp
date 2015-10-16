
#pragma once

namespace mgbase {

template <typename T>
class scoped_ptr;/* {
public:
    explicit scoped_ptr() : 
    
};*/

template <typename T>
class scoped_ptr<T []>
    : noncopyable
{
public:
    scoped_ptr() MGBASE_NOEXCEPT
        : ptr_(MGBASE_NULLPTR) { }
    
    explicit scoped_ptr(T* ptr) MGBASE_NOEXCEPT
        : ptr_(ptr) { }
    
    ~scoped_ptr()
    {
        reset();
    }
    
    void reset(T* ptr = MGBASE_NULLPTR) MGBASE_NOEXCEPT
    {
        delete[] ptr_;
        ptr_ = ptr;
    }
    
    scoped_ptr& operator = (T* ptr) MGBASE_NOEXCEPT
    {
        reset(ptr);
        return *this;
    }
    
    T& operator[] (std::ptrdiff_t index) const MGBASE_NOEXCEPT {
        return ptr_[index];
    }
    
    T* get() const MGBASE_NOEXCEPT {
        return ptr_;
    }
    
private:
    T* ptr_;
};

}

