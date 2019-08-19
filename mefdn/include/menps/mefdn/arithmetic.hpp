
#pragma once

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

template <typename T>
inline T floor_log2(T x) noexcept {
    MEFDN_STATIC_ASSERT_MSG(mefdn::is_integral<T>::value, "T must be integer");
    T result = 0;
    while ((x >>= 1) > 0)
        ++result;
    
    return result;
}

template <typename T>
constexpr inline bool is_power_of_2(const T x) {
    return x != 0 && ((x & (x-1)) == 0);
}

} // namespace mefdn
} // namespace menps

