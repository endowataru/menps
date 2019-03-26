
#pragma once

#include "integral_constant.hpp"
#include "sfinae.hpp"
#include <mgbase/utility/declval.hpp>

namespace mgbase {

namespace detail {

template <typename From, typename To>
struct is_convertible_impl
{
    static no_type  check(any_conversion, ...);
    static yes_type check(To, int);
    
    static const bool value = sizeof(check(declval<From>(), 0)) == sizeof(yes_type);
};

} // namespace detail

template <typename From, typename To>
struct is_convertible
    : bool_constant<detail::is_convertible_impl<From, To>::value> { };

} // namespace mgbase

