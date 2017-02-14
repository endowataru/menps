
#pragma once

#include <mgbase/atomic.hpp>

namespace mgult {

template <typename T>
class async_atomic_channel
{
public:
    async_atomic_channel()
        : ready_{false}
    { }
    
    void set_value(const T& val)
    {
        this->val_ = val;
        
        this->ready_.store(true, mgbase::memory_order_release);
    }
    
    template <typename Func>
    T& get(Func func)
    {
        while (! this->ready_.load(mgbase::memory_order_acquire))
        {
            func();
        }
        
        return this->val_;
    }
    
private:
    mgbase::atomic<bool>    ready_;
    T                       val_;
};

template <>
class async_atomic_channel<void>
{
public:
    async_atomic_channel()
        : ready_{false}
    { }
    
    void set_value()
    {
        this->ready_.store(true, mgbase::memory_order_release);
    }
    
    template <typename Func>
    void get(Func func)
    {
        while (! this->ready_.load(mgbase::memory_order_acquire))
        {
            func();
        }
    }
    
private:
    mgbase::atomic<bool>    ready_;
};

} // namespace mgult

