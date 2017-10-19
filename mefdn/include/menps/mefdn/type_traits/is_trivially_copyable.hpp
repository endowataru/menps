
#pragma once

#include <menps/mefdn/type_traits/integral_constant.hpp>

namespace menps {
namespace mefdn {

template <typename T>
struct is_trivially_copyable
    : bool_constant<(__has_trivial_copy(T))> { };

} // namespace mefdn
} // namespace menps

