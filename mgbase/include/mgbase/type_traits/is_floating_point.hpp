
#pragma once

#include "integral_constant.hpp"
#include "remove_cv.hpp"

namespace mgbase {

namespace detail {

template <typename> struct is_floating_point_helper      : false_type { };
template <> struct is_floating_point_helper<float>       : true_type { };
template <> struct is_floating_point_helper<double>      : true_type { };
template <> struct is_floating_point_helper<long double> : true_type { };

} // namespace detail

template <typename T>
struct is_floating_point
    : detail::is_floating_point_helper<typename remove_cv<T>::type> { };

} // namespace mgbase

