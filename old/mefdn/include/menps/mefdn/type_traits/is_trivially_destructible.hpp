
#pragma once

#include "integral_constant.hpp"

namespace menps {
namespace mefdn {

template <typename T>
struct is_trivially_destructible
    : bool_constant<(__has_trivial_destructor(T))> { };

} // namespace mefdn
} // namespace menps

