
#pragma once

#include <mgbase/type_traits.hpp>

namespace mgbase {

template <typename T, typename U>
MGBASE_ALWAYS_INLINE
typename mgbase::enable_if<
    !(mgbase::is_pointer<T>::value || mgbase::is_pointer<U>::value)
,   T
>::type
force_reinterpret_cast(const U& value) MGBASE_NOEXCEPT
{
    return static_cast<T>(value);
}

template <typename T, typename U>
MGBASE_ALWAYS_INLINE
typename mgbase::enable_if<
    (mgbase::is_pointer<T>::value || mgbase::is_pointer<U>::value)
,   T
>::type
force_reinterpret_cast(const U& value) MGBASE_NOEXCEPT
{
    return reinterpret_cast<T>(value);
}

} // namespace mgbase

