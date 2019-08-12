
#pragma once

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

namespace /*unnamed*/ {

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


template <typename T, T... Is>
struct static_max;

template <typename T, T I, T... Is>
struct static_max<T, I, Is...>
    : integral_constant<T,
        ((I < static_max<T, Is...>::value) ? static_max<T, Is...>::value : I)>
{ };

template <typename T, T I>
struct static_max<T, I>
    : integral_constant<T, I> { };

} // namespace mefdn
} // namespace menps

