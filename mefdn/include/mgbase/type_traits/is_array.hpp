
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

template <typename T>
struct is_array
    : false_type { };

template <typename T>
struct is_array<T []>
    : true_type { };

template <typename T, mgbase::size_t S>
struct is_array<T [S]>
    : true_type { };

} // namespace mgbase

