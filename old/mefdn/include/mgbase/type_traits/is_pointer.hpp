
#pragma once

#include "integral_constant.hpp"
#include "remove_cv.hpp"

namespace mgbase {

namespace detail {

template <typename T> struct is_pointer_helper     : false_type { };
template <typename T> struct is_pointer_helper<T*> : true_type  { };

} // namespace detail

template <typename T>
struct is_pointer
    : detail::is_pointer_helper<typename remove_cv<T>::type> { };

} // namespace mgbase

