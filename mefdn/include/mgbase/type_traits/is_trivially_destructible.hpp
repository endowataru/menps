
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

template <typename T>
struct is_trivially_destructible
    : bool_constant<(__has_trivial_destructor(T))> { };

} // namespace mgbase

