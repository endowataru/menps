
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

template <typename T>
struct is_trivially_copyable
    : bool_constant<(__has_trivial_copy(T))> { };

} // namespace mgbase

