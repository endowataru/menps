
#pragma once

#include <mgbase/move.hpp>

namespace mgbase {

template <typename T>
class value_wrapper
{
public:
    value_wrapper() MGBASE_EMPTY_DEFINITION
    
    MGBASE_CONSTEXPR  /*implicit*/ value_wrapper(const T& val)
        : val_(val) { }
    
    #ifdef MGBASE_CPP11_SUPPORTED
    MGBASE_CONSTEXPR /*implicit*/ value_wrapper(T&& val)
        : val_(mgbase::move(val)) { }
    #endif
    
    T& get() {
        return val_;
    }
    
    const T& get() const MGBASE_NOEXCEPT {
        return val_;
    }
    
    void assign_to(T* dest) const {
        *dest = val_;
    }

private:
    T val_;
};

template <typename T>
class value_wrapper<T&>
{
public:
    value_wrapper() MGBASE_EMPTY_DEFINITION
    
    /*implicit*/ value_wrapper(T& val)
        : val_(&val) { }
    
    T& get() const MGBASE_NOEXCEPT {
        return *val_;
    }
    
    operator T&() const MGBASE_NOEXCEPT {
        return get();
    }

private:
    T* val_;
};

template <>
class value_wrapper<void>
{
public:
    void get() const MGBASE_NOEXCEPT { }
    
    void assign_to(void* /*dest*/) const {
        // do nothing
    }
};

namespace /*unnamed*/ {

inline value_wrapper<void> wrap_void() MGBASE_NOEXCEPT {
    return value_wrapper<void>();
}
template <typename T>
inline value_wrapper<T> wrap_value(T& value) {
    return value_wrapper<T>(value);
}
template <typename T>
inline value_wrapper<T> wrap_value(const T& value) {
    return value_wrapper<T>(value);
}

template <typename T>
inline value_wrapper<T&> wrap_reference(T& value) MGBASE_NOEXCEPT {
    return value_wrapper<T&>(value);
}
template <typename T>
inline value_wrapper<const T&> wrap_reference(const T& value) MGBASE_NOEXCEPT {
    return value_wrapper<const T&>(value);
}

} // unnamed namespace

namespace detail {

template <typename R>
struct call_with_value_wrapper_helper
{
    template <typename Func, typename T>
    static value_wrapper<R> call(Func func, value_wrapper<T>& val)
    {
        return func(val.get());
    }
    template <typename Func, typename T>
    static value_wrapper<R> call(Func func, const value_wrapper<T>& val)
    {
        return func(val.get());
    }
    template <typename Func>
    static value_wrapper<R> call(Func func, value_wrapper<void>& /*val*/)
    {
        return func();
    }
    template <typename Func>
    static value_wrapper<R> call(Func func, const value_wrapper<void>& /*val*/)
    {
        return func();
    }
};

template <>
struct call_with_value_wrapper_helper<void>
{
    template <typename Func, typename T>
    static value_wrapper<void> call(Func func, value_wrapper<T>& val)
    {
        func(val.get());
        return wrap_void();
    }
    template <typename Func, typename T>
    static value_wrapper<void> call(Func func, const value_wrapper<T>& val)
    {
        func(val.get());
        return wrap_void();
    }
    template <typename Func>
    static value_wrapper<void> call(Func func, value_wrapper<void>& /*val*/)
    {
        func();
        return wrap_void();
    }
    template <typename Func>
    static value_wrapper<void> call(Func func, const value_wrapper<void>& /*val*/)
    {
        func();
        return wrap_void();
    }
};

} // namespace detail

namespace /*unnamed*/ {

// R must be specified due to the lack of decltype in C++03
template <typename R, typename Func, typename T>
inline value_wrapper<R> call_with_value_wrapper(Func func, value_wrapper<T>& val)
{
    return detail::call_with_value_wrapper_helper<R>::call(func, val);
}
template <typename R, typename Func, typename T>
inline value_wrapper<R> call_with_value_wrapper(Func func, const value_wrapper<T>& val)
{
    return detail::call_with_value_wrapper_helper<R>::call(func, val);
}

} // unnamed namespace


namespace detail {

template <typename R>
struct call_with_value_wrapper_2_helper
{
    template <typename Func, typename Arg1, typename T>
    MGBASE_ALWAYS_INLINE static value_wrapper<R> call(Func func, Arg1 arg1, value_wrapper<T>& val)
    {
        return func(arg1, val.get());
    }
    template <typename Func, typename Arg1, typename T>
    MGBASE_ALWAYS_INLINE static value_wrapper<R> call(Func func, Arg1 arg1, const value_wrapper<T>& val)
    {
        return func(arg1, val.get());
    }
    template <typename Func, typename Arg1>
    MGBASE_ALWAYS_INLINE static value_wrapper<R> call(Func func, Arg1 arg1, value_wrapper<void>& /*val*/)
    {
        return func(arg1);
    }
    template <typename Func, typename Arg1>
    MGBASE_ALWAYS_INLINE static value_wrapper<R> call(Func func, Arg1 arg1, const value_wrapper<void>& /*val*/)
    {
        return func(arg1);
    }
};

template <>
struct call_with_value_wrapper_2_helper<void>
{
    template <typename Func, typename Arg1, typename T>
    MGBASE_ALWAYS_INLINE static value_wrapper<void> call(Func func, Arg1 arg1, value_wrapper<T>& val)
    {
        func(arg1, val.get());
        return wrap_void();
    }
    template <typename Func, typename Arg1, typename T>
    MGBASE_ALWAYS_INLINE static value_wrapper<void> call(Func func, Arg1 arg1, const value_wrapper<T>& val)
    {
        func(arg1, val.get());
        return wrap_void();
    }
    template <typename Func, typename Arg1>
    MGBASE_ALWAYS_INLINE static value_wrapper<void> call(Func func, Arg1 arg1, value_wrapper<void>& /*val*/)
    {
        func(arg1);
        return wrap_void();
    }
    template <typename Func, typename Arg1>
    MGBASE_ALWAYS_INLINE static value_wrapper<void> call(Func func, Arg1 arg1, const value_wrapper<void>& /*val*/)
    {
        func(arg1);
        return wrap_void();
    }
};

} // namespace detail

namespace /*unnamed*/ {

// R must be specified due to the lack of decltype in C++03
template <typename R, typename Func, typename Arg1, typename T>
MGBASE_ALWAYS_INLINE value_wrapper<R> call_with_value_wrapper_2(Func func, Arg1 arg1, value_wrapper<T>& val)
{
    return detail::call_with_value_wrapper_2_helper<R>::call(func, arg1, val);
}
template <typename R, typename Func, typename Arg1, typename T>
MGBASE_ALWAYS_INLINE value_wrapper<R> call_with_value_wrapper_2(Func func, Arg1 arg1, const value_wrapper<T>& val)
{
    return detail::call_with_value_wrapper_2_helper<R>::call(func, arg1, val);
}

} // unnamed namespace

} // namespace mgbase

