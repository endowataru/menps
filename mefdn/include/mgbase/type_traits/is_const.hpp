
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

template <typename T> struct is_const          : false_type {};
template <typename T> struct is_const<const T> : true_type  {};

} // namespace mgbase

