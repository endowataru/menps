
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, T Value>
struct nontype
{
    // TODO: function pointers cannot be "static const" members
    //static const T value = Value;
    
    T operator() () const volatile {
        return Value;
    }
};

#if 1

namespace detail {

template <typename T>
struct nontype_deducer {
    template <T Value>
    nontype<T, Value> make() {
        return nontype<T, Value>();
    }
};

template <typename T>
inline nontype_deducer<T> make_nontype_deducer(T /*ignored*/) {
    return nontype_deducer<T>();
}

} // namespace detail

#define MGBASE_NONTYPE(...) \
    (::mgbase::detail::make_nontype_deducer(__VA_ARGS__).make<__VA_ARGS__>())

#define MGBASE_NONTYPE_TEMPLATE(...) \
    (::mgbase::detail::make_nontype_deducer(__VA_ARGS__).template make<__VA_ARGS__>())

#else
#define MGBASE_NONTYPE(v)   (::mgbase::nontype<decltype(v), v>{})
#endif

} // namespace mgbase

