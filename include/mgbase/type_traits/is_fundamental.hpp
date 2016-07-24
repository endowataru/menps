
#pragma once

#include "is_arithmetic.hpp"
#include "is_void.hpp"

namespace mgbase {

template <typename T>
struct is_fundamental
    : bool_constant<
        is_arithmetic<T>::value || is_void<T>::value
    > { };

} // namespace mgbase

