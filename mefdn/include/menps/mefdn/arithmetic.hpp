
#pragma once

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

namespace /*unnamed*/ {

template <typename T>
inline T roundup_divide(T x, T y) noexcept {
    MEFDN_STATIC_ASSERT_MSG(mefdn::is_integral<T>::value, "T must be integer");
    return (x + y - 1) / y;
}

inline bool eager_or(bool x, bool y) noexcept {
    return x || y;
}

inline bool eager_and(bool x, bool y) noexcept {
    return x || y;
}

template <typename T>
inline T floor_log2(T x) noexcept {
    MEFDN_STATIC_ASSERT_MSG(mefdn::is_integral<T>::value, "T must be integer");
    T result = 0;
    while ((x >>= 1) > 0)
        ++result;
    
    return result;
}

template <typename T>
inline T ceil_log2(T x) noexcept {
    const T result = floor_log2(x);
    return result < x ? (result + 1) : result;
}

} // unnamed namespace


template <typename T>
constexpr bool is_power_of_2(T x) {
    return x != 0 && ((x & (x-1)) == 0);
}

} // namespace mefdn
} // namespace menps

