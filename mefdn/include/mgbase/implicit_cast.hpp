
#pragma once

#include "type_traits.hpp"
#include <mgbase/utility/forward.hpp>

namespace mgbase {

// Restricted implicit_cast
// Referred from: http://d.hatena.ne.jp/gintenlabo/20130419/1366378612
template <typename To,
    typename = typename enable_if<
        is_convertible<To&&, To>::value
    >::type
>
To implicit_cast(typename enable_if<true, To>::type&& x) {
    return forward<To>(x);
}

template <typename To, typename From,
    typename = typename enable_if<
        ! is_reference<To>::value &&
        is_convertible<From, To>::value
    >::type
>
inline To implicit_cast(From&& x) {
    return forward<From>(x);
}

} // namespace mgbase

