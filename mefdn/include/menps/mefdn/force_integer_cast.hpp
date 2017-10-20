
#pragma once

#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

template <typename T, typename U>
typename mefdn::enable_if<
    ! mefdn::is_pointer<U>::value
,   T
>::type
force_integer_cast(const U& value) noexcept
    // TODO: is this really "noexcept"?
{
    return static_cast<T>(value);
}

template <typename T, typename U>
typename mefdn::enable_if<
    mefdn::is_pointer<U>::value
,   T
>::type
force_integer_cast(const U& value) noexcept
{
    return reinterpret_cast<T>(value);
}

} // namespace mefdn
} // namespace menps

