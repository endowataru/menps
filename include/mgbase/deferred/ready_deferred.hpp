
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
class ready_deferred
{
public:
    ready_deferred() MGBASE_EMPTY_DEFINITION
    
    /*implicit*/ ready_deferred(const T& val)
        : val_(val) { }
    
    T& get() {
        return val_;
    }
    
    const T& get() const MGBASE_NOEXCEPT {
        return val_;
    }
    
private:
    T val_;
};

template <>
class ready_deferred<void>
{
public:
    void get() { }
};

namespace /*unnamed*/ {

inline MGBASE_ALWAYS_INLINE ready_deferred<void> make_ready_deferred() MGBASE_NOEXCEPT {
    return ready_deferred<void>();
}

template <typename T>
inline MGBASE_ALWAYS_INLINE ready_deferred<T> make_ready_deferred(const T& val) {
    return ready_deferred<T>(val);
}

} // unnamed namespace

namespace detail {

namespace /*unnamed*/ {

template <typename R>
struct call_and_make_ready_impl
{
    template <typename Func, typename T>
    static MGBASE_ALWAYS_INLINE ready_deferred<R> f(Func func, const ready_deferred<T>& val) {
        return func(val.get());
    }
    
    template <typename Func>
    static MGBASE_ALWAYS_INLINE ready_deferred<R> f(Func func, const ready_deferred<void>& /*val*/) {
        return func();
    }
};

template <>
struct call_and_make_ready_impl<void>
{
    template <typename Func, typename T>
    static MGBASE_ALWAYS_INLINE ready_deferred<void> f(Func func, const ready_deferred<T>& val) {
        func(val.get());
        return make_ready_deferred();
    }
    
    template <typename Func>
    static MGBASE_ALWAYS_INLINE ready_deferred<void> f(Func func, const ready_deferred<void>& /*val*/) {
        func();
        return make_ready_deferred();
    }
};

} // unnamed namespace

} // namespace detail

namespace /*unnamed*/ {

template <typename R, typename Func, typename T>
inline MGBASE_ALWAYS_INLINE ready_deferred<R> call_and_make_ready(Func func, const ready_deferred<T>& val)
{
    return detail::call_and_make_ready_impl<R>::f(func, val);
}

} // unnamed namespace

} // namespace mgbase

