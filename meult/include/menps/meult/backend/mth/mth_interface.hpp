
#pragma once

#include <menps/meult/backend/mth/thread.hpp>
#include <menps/meult/backend/mth/mutex.hpp>
#include <menps/meult/backend/mth/condition_variable.hpp>
#include <menps/meult/backend/mth/this_thread.hpp>
#include <menps/meult/backend/mth/thread_specific.hpp>
#include <menps/meult/backend/mth/for_loop.hpp>
#include <menps/meult/backend/mth/uncond_variable.hpp>

#include <menps/meult/klt.hpp>

namespace menps {
namespace meult {
namespace backend {
namespace mth {

#if 0

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
        mefdn::unique_lock<mutex> lk(this->mtx);
        while (!this->is_ready) {
            cv.wait(lk);
        }
    }
    void get() {
        this->wait();
    }
    
    void notify()
    {
        mefdn::unique_lock<mutex> lk(this->mtx);
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
        return mefdn::move(this->val);
    }
};

template <typename T>
struct notify_cond_channel
{
    cond_channel<T>& ch;
    
    template <typename U>
    void operator() (U&& val) const /*may throw*/
    {
        ch.val = mefdn::forward<U>(val);
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
        mefdn::forward<Func>(func)(
            mefdn::forward<Args>(args)...
        ,   detail::notify_cond_channel<T>{ ch }
        );
    
    if (d.is_ready()) {
        return async_get(mefdn::move(d));
    }
    else {
        return ch.get();
    }
}

#endif

using mefdn::unique_lock;
using mefdn::lock_guard;

inline mefdn::size_t get_num_workers() noexcept
{
    return static_cast<mefdn::size_t>(myth_get_num_workers());
}

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

