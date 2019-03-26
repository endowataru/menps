
#pragma once

#include "is_fundamental.hpp"

namespace mgbase {

template <typename T>
struct is_compound
    : bool_constant<
        !is_fundamental<T>::value
    > { };

} // namespace mgbase

