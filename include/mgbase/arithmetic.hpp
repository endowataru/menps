
#pragma once

#include <mgbase/type_traits.hpp>

namespace mgbase {

namespace /*unnamed*/ {

template <typename T>
inline T roundup_divide(T x, T y) MGBASE_NOEXCEPT {
    MGBASE_STATIC_ASSERT(mgbase::is_integral<T>::value, "T must be integer");
    return (x + y - 1) / y;
}

inline bool eager_or(bool x, bool y) MGBASE_NOEXCEPT {
    return x || y;
}

inline bool eager_and(bool x, bool y) MGBASE_NOEXCEPT {
    return x || y;
}

template <typename T>
inline T floor_log2(T x) MGBASE_NOEXCEPT {
    MGBASE_STATIC_ASSERT(mgbase::is_integral<T>::value, "T must be integer");
    T result = 0;
    while ((x >>= 1) > 0)
        ++result;
    
    return result;
}

template <typename T>
inline T ceil_log2(T x) MGBASE_NOEXCEPT {
    const T result = floor_log2(x);
    return result < x ? (result + 1) : result;
}

} // unnamed namespace

} // namespace mgbase

