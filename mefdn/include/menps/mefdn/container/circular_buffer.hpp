
#pragma once

#include <menps/mefdn/scoped_ptr.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace mefdn {

template <typename Derived, typename T>
class circular_buffer_base
{
public:
    typedef mefdn::size_t  size_type;
    
    size_type size() const noexcept {
        return size_;
    }
    
    bool empty() const noexcept {
        const bool ret = size() == 0;
        
        // Error if: (ret && (first_ != last_))
        MEFDN_ASSERT(!ret || (first_ == last_));
        
        return ret;
    }
    bool full() const noexcept {
        const bool ret = size() == capacity();
        
        // Error if: (ret && (first_ != last_))
        MEFDN_ASSERT(!ret || (first_ == last_));
        
        return ret;
    }
    
    T& front() noexcept {
        MEFDN_ASSERT(!empty());
        return data()[front_index()];
    }
    const T& front() const noexcept {
        MEFDN_ASSERT(!empty());
        return data()[front_index()];
    }
    
    T& back() noexcept {
        MEFDN_ASSERT(!empty());
        return data()[back_index()];
    }
    const T& back() const noexcept {
        MEFDN_ASSERT(!empty());
        return data()[back_index()];
    }
    
    void pop_front()
    {
        MEFDN_ASSERT(!empty());
        --size_;
        
        increment(&first_);
    }
    void pop_back()
    {
        MEFDN_ASSERT(!empty());
        --size_;
        
        decrement(&last_);
    }
    
    void push_front(const T& val)
    {
        MEFDN_ASSERT(!full());
        ++size_;
        
        decrement(&first_);
        
        // Copy the value after decrement.
        data()[first_] = val;
    }
    void push_back(const T& val)
    {
        MEFDN_ASSERT(!full());
        ++size_;
        
        // Copy the value before increment.
        data()[last_] = val;
        
        increment(&last_);
    }
    
    void push_back()
    {
        MEFDN_ASSERT(!full());
        ++size_;
        
        increment(&last_);
    }
    
    void erase_begin(const size_type n)
    {
        for (size_type i = 0; i < n; ++i)
            pop_front();
    }
    void erase_end(const size_type n)
    {
        for (size_type i = 0; i < n; ++i)
            pop_back();
    }
    
    T* raw_data() noexcept {
        return data();
    }
    const T* raw_data() const noexcept {
        return data();
    }
    
protected:
    circular_buffer_base()
        : first_(0), last_(0), size_(0) { }

private:
    size_type front_index() const noexcept {
        return first_;
    }
    size_type back_index() const noexcept {
        size_type index = last_;
        decrement(&index);
        return index;
    }
    
    void increment(size_type* const index) const noexcept {
        ++*index;
        if (*index == capacity())
            *index = 0;
    }
    void decrement(size_type* const index) const noexcept {
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
    
          T* data()       noexcept { return derived().data(); }
    const T* data() const noexcept { return derived().data(); }
    
    mefdn::size_t capacity() const noexcept { return derived().capacity(); }
    
          Derived& derived()       noexcept { return static_cast<      Derived&>(*this); }
    const Derived& derived() const noexcept { return static_cast<const Derived&>(*this); }
};

template <typename T>
class circular_buffer
    : public circular_buffer_base<circular_buffer<T>, T>
{
    typedef circular_buffer_base<circular_buffer<T>, T> base;
    
    typedef typename base::size_type    size_type;
    
public:
    
    circular_buffer() : ptr_(), capacity_(0) { }
    
    explicit circular_buffer(const size_type cap)
    {
        set_capacity(cap);
    }
    
    ~circular_buffer() MEFDN_EMPTY_DEFINITION
    
    void set_capacity(size_type cap)
    {
        // TODO: Moving elements is not supported yet.
        MEFDN_ASSERT(ptr_ == nullptr);
        
        ptr_ = new T[cap];
        capacity_ = cap;
    }

private:
    friend class circular_buffer_base<circular_buffer<T>, T>;
    
    T* data() const noexcept { return ptr_.get(); }
    size_type capacity() const noexcept { return capacity_; }
    
    mefdn::scoped_ptr<T []> ptr_;
    size_type capacity_;
};

template <typename T, mefdn::size_t Capacity>
class static_circular_buffer
    : public circular_buffer_base<static_circular_buffer<T, Capacity>, T>
{
    typedef circular_buffer_base<static_circular_buffer, T> base;
    
private:
    friend class circular_buffer_base<static_circular_buffer, T>;
    
          T* data()       noexcept { return arr_; }
    const T* data() const noexcept { return arr_; }
    
    mefdn::size_t capacity() const noexcept { return Capacity; }
    
    T arr_[Capacity];
};

} // namespace mefdn
} // namespace menps

