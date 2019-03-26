
#pragma once

#include "integral_constant.hpp"
#include "remove_cv.hpp"

namespace mgbase {

namespace detail {

template <typename>
struct is_member_pointer_helper
    : mgbase::false_type {};
 
template <typename T, typename U>
struct is_member_pointer_helper<T U::*>
    : mgbase::true_type {};

} // namespace detail
 
template <typename T>
struct is_member_pointer
    : detail::is_member_pointer_helper<
        typename mgbase::remove_cv<T>::type
    > {};

} // namespace mgbase

