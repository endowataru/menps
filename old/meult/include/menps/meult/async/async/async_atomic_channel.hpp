
#pragma once

#include <menps/mefdn/atomic.hpp>

namespace menps {
namespace meult {

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
        this->val_ = mefdn::forward<U>(val);
        
        this->ready_.store(true, mefdn::memory_order_release);
    }
    
    template <typename Func>
    T& get(Func func)
    {
        while (! this->ready_.load(mefdn::memory_order_acquire))
        {
            func();
        }
        
        return this->val_;
    }
    
private:
    mefdn::atomic<bool>    ready_;
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
        this->ready_.store(true, mefdn::memory_order_release);
    }
    
    template <typename Func>
    void get(Func func) const
    {
        while (! this->ready_.load(mefdn::memory_order_acquire))
        {
            func();
        }
    }
    
private:
    mefdn::atomic<bool>    ready_;
};

// For old GCC, we cannot put these functions
// as member functions in async_atomic_channel<t>.
template <typename T, typename Func>
inline T& async_get(async_atomic_channel<T>& ch, Func&& func) {
    return ch.get(mefdn::forward<Func>(func));
}
template <typename T, typename Func>
inline T&& async_get(async_atomic_channel<T>&& ch, Func&& func) {
    return mefdn::move(ch.get(mefdn::forward<Func>(func)));
}
template <typename Func>
inline void async_get(const async_atomic_channel<void>& ch, Func&& func) {
    return ch.get(mefdn::forward<Func>(func));
}

} // namespace meult
} // namespace menps

