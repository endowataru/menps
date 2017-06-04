
#pragma once

#include <mgult/common.hpp>
#include <mgbase/thread.hpp>
#include <mgbase/mutex.hpp>
#include <mgbase/condition_variable.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/sync_flag.hpp>

#include <mgbase/type_traits/decay.hpp>

#include <mgult/async/async_status.hpp>
#include <mgult/async/async_atomic_channel.hpp>

namespace mgult {
namespace klt {

using mgbase::mutex;
using mgbase::condition_variable;

using mgbase::lock_guard;
using mgbase::unique_lock;

using mgbase::thread;

namespace this_thread {

#ifdef MGBASE_ULT_DISABLE_YIELD
inline void yield() MGBASE_NOEXCEPT { }
#else
using mgbase::this_thread::yield;
#endif

inline void detach()
{
    pthread_detach(pthread_self());
}

} // namespace this_thread

using mgbase::spinlock;

using mgult::async_status;
using mgult::make_async_ready;
using mgult::make_async_deferred;

namespace detail {

template <typename T>
struct notify_channel
{
    async_atomic_channel<T>*    ch;
    
    template <typename U>
    void operator() (U&& val) const /*may throw*/ {
        ch->set_value(mgbase::forward<U>(val));
    }
};
template <>
struct notify_channel<void>
{
    async_atomic_channel<void>* ch;
    
    void operator() () const MGBASE_NOEXCEPT {
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
        mgbase::forward<Func>(func)(
            mgbase::forward<Args>(args)...
        ,   cont
        );
    
    if (d.is_ready()) {
        return async_get(mgbase::move(d));
    }
    else {
        return async_get(mgbase::move(ch), klt::this_thread::yield);
    }
}

using mgbase::sync_flag;

inline mgbase::size_t get_num_workers() MGBASE_NOEXCEPT {
    return -1;
}

} // namespace klt
} // namespace mgult

