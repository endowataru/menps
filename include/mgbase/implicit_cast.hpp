
#pragma once

#include "type_traits.hpp"

namespace mgbase {

#if (__cplusplus < 201103L)

template <typename T>
inline T implicit_cast(typename identity<T>::type x) {
    return x;
}

#else

// Restricted implicit_cast
// Referred from: http://d.hatena.ne.jp/gintenlabo/20130419/1366378612
template <typename To,
    typename = typename std::enable_if<
        std::is_convertible<To&&, To>::value
    >::type
>
To implicit_cast(typename std::enable_if<true, To>&& x) {
    return std::forward<To>(x);
}

template <typename To, typename From,
    typename = typename std::enable_if<
        !std::is_reference<To>::value &&
        std::is_convertible<From, To>::value
    >
>
To implicit_cast(From&& x) {
    return std::forward<From>(x);
}

#endif

}

