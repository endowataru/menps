
#pragma once

#include <mgbase/scoped_ptr.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

template <typename Derived, typename T>
class circular_buffer_base
{
public:
    typedef mgbase::size_t  size_type;
    
    size_type size() const MGBASE_NOEXCEPT {
        return size_;
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        const bool ret = size() == 0;
        
        // Error if: (ret && (first_ != last_))
        MGBASE_ASSERT(!ret || (first_ == last_));
        
        return ret;
    }
    bool full() const MGBASE_NOEXCEPT {
        const bool ret = size() == capacity();
        
        // Error if: (ret && (first_ != last_))
        MGBASE_ASSERT(!ret || (first_ == last_));
        
        return ret;
    }
    
    T& front() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(!empty());
        return data()[front_index()];
    }
    const T& front() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(!empty());
        return data()[front_index()];
    }
    
    T& back() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(!empty());
        return data()[back_index()];
    }
    const T& back() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(!empty());
        return data()[back_index()];
    }
    
    void pop_front()
    {
        MGBASE_ASSERT(!empty());
        --size_;
        
        increment(&first_);
    }
    void pop_back()
    {
        MGBASE_ASSERT(!empty());
        --size_;
        
        decrement(&last_);
    }
    
    void push_front(const T& val)
    {
        MGBASE_ASSERT(!full());
        ++size_;
        
        decrement(&first_);
        
        // Copy the value after decrement.
        data()[first_] = val;
    }
    void push_back(const T& val)
    {
        MGBASE_ASSERT(!full());
        ++size_;
        
        // Copy the value before increment.
        data()[last_] = val;
        
        increment(&last_);
    }

private:
    size_type front_index() const MGBASE_NOEXCEPT {
        return first_;
    }
    size_type back_index() const MGBASE_NOEXCEPT {
        size_type index = last_;
        decrement(&index);
        return index;
    }
    
    void increment(size_type* const index) const MGBASE_NOEXCEPT {
        ++*index;
        if (*index == capacity())
            *index = 0;
    }
    void decrement(size_type* const index) const MGBASE_NOEXCEPT {
        if (*index == 0)
            *index = capacity() - 1;
        else
            --*index;
    }
    
    // Valid elements are in the range [first_, last_).
    // States:
    //  - empty : first_ == last_ && size_ == 0
    //  - full  : first_ == last_ && size_ == capaicty()
    //  - otherwise: first_ != last_ && (0 < size_ || size_ < capacity())
    size_type first_;
    size_type last_;
    size_type size_;
    
          T* data()       MGBASE_NOEXCEPT { return derived().data(); }
    const T* data() const MGBASE_NOEXCEPT { return derived().data(); }
    
    mgbase::size_t capacity() const MGBASE_NOEXCEPT { return derived().capacity(); }
    
          Derived& derived()       MGBASE_NOEXCEPT { return static_cast<      Derived&>(*this); }
    const Derived& derived() const MGBASE_NOEXCEPT { return static_cast<const Derived&>(*this); }
};

template <typename T>
class circular_buffer
    : public circular_buffer_base<circular_buffer<T>, T>
{
    typedef circular_buffer_base<circular_buffer<T>, T> base;
    
public:
    using typename base::size_type;
    
    circular_buffer() : ptr_(), capacity_(0) { }
    
    explicit circular_buffer(const size_type capacity)
    {
        set_capacity(capacity);
    }
    
    ~circular_buffer() MGBASE_EMPTY_DEFINITION
    
    void set_capacity(size_type cap)
    {
        // TODO: Moving elements is not supported yet.
        MGBASE_ASSERT(ptr_ == MGBASE_NULLPTR);
        
        ptr_ = new T[cap];
        capacity_ = cap;
    }

private:
    friend class circular_buffer_base<circular_buffer<T>, T>;
    
    T* data() const MGBASE_NOEXCEPT { return ptr_.get(); }
    size_type capacity() const MGBASE_NOEXCEPT { return capacity_; }
    
    mgbase::scoped_ptr<T []> ptr_;
    size_type capacity_;
};

template <typename T, mgbase::size_t Capacity>
class static_circular_buffer
    : public circular_buffer_base<static_circular_buffer<T, Capacity>, T>
{
    typedef circular_buffer_base<static_circular_buffer, T> base;
    
private:
    friend class circular_buffer_base<static_circular_buffer, T>;
    
          T* data()       MGBASE_NOEXCEPT { return arr_; }
    const T* data() const MGBASE_NOEXCEPT { return arr_; }
    
    mgbase::size_t capacity() const MGBASE_NOEXCEPT { return Capacity; }
    
    T arr_[Capacity];
};

} // namespace mgbase

