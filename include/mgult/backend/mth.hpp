
#pragma once

#include <mgult/backend/mth/thread.hpp>
#include <mgult/backend/mth/mutex.hpp>
#include <mgult/backend/mth/condition_variable.hpp>

#include <mgult/async/async_status.hpp>
#include <mgult/async/async_atomic_channel.hpp>

#include <mgult/klt.hpp>

namespace mgult {
namespace backend {
namespace mth {

using mgult::async_status;

namespace detail {

template <typename T>
struct cond_channel;

template <>
struct cond_channel<void>
{
    mutex               mtx;
    condition_variable  cv;
    bool                is_ready;
    
    cond_channel()
        : mtx()
        , cv()
        , is_ready(false)
    {}
    
    void wait()
    {
        mgbase::unique_lock<mutex> lk(this->mtx);
        while (!this->is_ready) {
            cv.wait(lk);
        }
    }
    void get() {
        this->wait();
    }
    
    void notify()
    {
        mgbase::unique_lock<mutex> lk(this->mtx);
        this->is_ready = true;
        this->cv.notify_one();
    }
};
template <typename T>
struct cond_channel
    : cond_channel<void>
{
    T   val;
    
    // Hide cond_channel<void>::get.
    T get() {
        this->wait();
        
        // TODO: "this" must be "rvalue"
        return mgbase::move(this->val);
    }
};

template <typename T>
struct notify_cond_channel
{
    cond_channel<T>& ch;
    
    template <typename U>
    void operator() (U&& val) const /*may throw*/
    {
        ch.val = mgbase::forward<U>(val);
        ch.notify();
    }
};
template <>
struct notify_cond_channel<void>
{
    cond_channel<void>& ch;
    
    void operator() () const /*may throw*/
    {
        ch.notify();
    }
};

} // namespace detail

template <typename T, typename Func, typename... Args>
inline T suspend_and_call(Func&& func, Args&&... args)
{
    detail::cond_channel<T> ch;
    
    auto&& d =
        mgbase::forward<Func>(func)(
            mgbase::forward<Args>(args)...
        ,   detail::notify_cond_channel<T>{ ch }
        );
    
    if (d.is_ready()) {
        return async_get(mgbase::move(d));
    }
    else {
        return ch.get();
    }
}

using klt::make_async_ready;
using klt::make_async_deferred;

namespace this_thread {

using klt::this_thread::yield;

} // namespace this_thread

using mgbase::unique_lock;
using mgbase::lock_guard;

using mgbase::sync_flag;

} // namespace mth
} // namespace backend
} // namespace mgult

