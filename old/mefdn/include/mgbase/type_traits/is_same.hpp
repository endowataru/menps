
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, typename U>
struct is_same : false_type {};

template <typename T>
struct is_same<T, T> : true_type {};

} // namespace mgbase

