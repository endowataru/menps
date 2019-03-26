
#pragma once

#include "integral_constant.hpp"
#include "is_lvalue_reference.hpp"
#include "is_rvalue_reference.hpp"

namespace mgbase {

template <typename T>
struct is_reference
    : bool_constant<(is_lvalue_reference<T>::value
        || is_rvalue_reference<T>::value)> { };

} // namespace mgbase

