
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
    
    template <typename U>
    void set_value(U&& val)
    {
        this->val_ = mgbase::forward<U>(val);
        
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
    void get(Func func) const
    {
        while (! this->ready_.load(mgbase::memory_order_acquire))
        {
            func();
        }
    }
    
private:
    mgbase::atomic<bool>    ready_;
};

// For old GCC, we cannot put these functions
// as member functions in async_atomic_channel<t>.
template <typename T, typename Func>
inline T& async_get(async_atomic_channel<T>& ch, Func&& func) {
    return ch.get(mgbase::forward<Func>(func));
}
template <typename T, typename Func>
inline T&& async_get(async_atomic_channel<T>&& ch, Func&& func) {
    return mgbase::move(ch.get(mgbase::forward<Func>(func)));
}
template <typename Func>
inline void async_get(const async_atomic_channel<void>& ch, Func&& func) {
    return ch.get(mgbase::forward<Func>(func));
}

} // namespace mgult

