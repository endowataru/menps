
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

template <typename Base, typename Derived>
struct is_base_of
    : bool_constant<__is_base_of(Base, Derived)> { };

} // namespace mgbase

