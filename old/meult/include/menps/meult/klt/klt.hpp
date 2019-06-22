
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/condition_variable.hpp>
#include <menps/mefdn/thread/spinlock.hpp>
#include <menps/mefdn/thread/barrier.hpp>

#include <menps/mefdn/type_traits.hpp>

#include <menps/meult/klt/emulated_uncond_variable.hpp>

namespace menps {
namespace meult {
namespace klt {

using mefdn::mutex;
using mefdn::condition_variable;

using mefdn::lock_guard;
using mefdn::unique_lock;

using mefdn::thread;

namespace this_thread {

#ifdef MEFDN_ULT_DISABLE_YIELD
inline void yield() noexcept { }
#else
using mefdn::this_thread::yield;
#endif

inline void detach()
{
    pthread_detach(pthread_self());
}

} // namespace this_thread

using mefdn::spinlock;

#if 0
namespace detail {

template <typename T>
struct notify_channel
{
    async_atomic_channel<T>*    ch;
    
    template <typename U>
    void operator() (U&& val) const /*may throw*/ {
        ch->set_value(mefdn::forward<U>(val));
    }
};
template <>
struct notify_channel<void>
{
    async_atomic_channel<void>* ch;
    
    void operator() () const noexcept {
        ch->set_value();
    }
};

} // namespace detail

template <typename T, typename Func, typename... Args>
inline T suspend_and_call(Func&& func, Args&&... args)
{
    async_atomic_channel<T> ch;
    
    detail::notify_channel<T> cont{ &ch };
    
    auto&& d =
        mefdn::forward<Func>(func)(
            mefdn::forward<Args>(args)...
        ,   cont
        );
    
    if (d.is_ready()) {
        return async_get(mefdn::move(d));
    }
    else {
        return async_get(mefdn::move(ch), klt::this_thread::yield);
    }
}
#endif

inline mefdn::size_t get_num_workers() noexcept {
    return std::thread::hardware_concurrency();
}


template <typename Policy>
class thread_specific
{
    typedef typename Policy::value_type value_type;
    
public:
    value_type* get() {
        return p_;
    }
    
    void set(value_type* const ptr) const {
        p_ = ptr;
    }
    
private:
    static MEFDN_THREAD_LOCAL value_type* p_;
};

template <typename Policy>
MEFDN_THREAD_LOCAL typename Policy::value_type* thread_specific<Policy>::p_ = nullptr;

using menps::mefdn::barrier;

} // namespace klt
} // namespace meult
} // namespace menps

