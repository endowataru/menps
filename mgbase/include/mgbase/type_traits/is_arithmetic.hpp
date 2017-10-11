
#pragma once

#include "integral_constant.hpp"
#include "is_integral.hpp"
#include "is_floating_point.hpp"

namespace mgbase {

template <typename T>
struct is_arithmetic
    : bool_constant<
        is_integral<T>::value || is_floating_point<T>::value
    > { };

} // namespace mgbase

