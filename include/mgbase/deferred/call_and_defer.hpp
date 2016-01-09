
#pragma once

#include "deferred.hpp"

namespace mgbase {

namespace detail {

namespace /*unnamed*/ {

template <typename R>
struct call_and_defer_impl
{
    template <typename Func, typename T>
    static MGBASE_ALWAYS_INLINE deferred<R> f(Func func, const ready_deferred<T>& val) {
        return func(val.get());
    }
    
    template <typename Func>
    static MGBASE_ALWAYS_INLINE deferred<R> f(Func func, const ready_deferred<void>& /*val*/) {
        return func();
    }
};

} // unnamed namespace

} // namespace detail

namespace /*unnamed*/ {

template <typename R, typename Func, typename T>
inline MGBASE_ALWAYS_INLINE deferred<R> call_and_defer(Func func, const ready_deferred<T>& val)
{
    return detail::call_and_defer_impl<R>::f(func, val);
}

} // unnamed namespace

} // namespace mgbase

