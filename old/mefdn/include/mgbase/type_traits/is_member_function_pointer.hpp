
#pragma once

#include "integral_constant.hpp"
#include "is_function.hpp"
#include "remove_cv.hpp"

namespace mgbase {

namespace detail {

template <typename>
struct is_member_function_pointer_helper
    : mgbase::false_type {};
 
template <typename T, typename U>
struct is_member_function_pointer_helper<T U::*>
    : mgbase::is_function<T> {};

} // namespace detail
 
template <typename T>
struct is_member_function_pointer
    : detail::is_member_function_pointer_helper<
        typename mgbase::remove_cv<T>::type
    > {};

} // namespace mgbase

