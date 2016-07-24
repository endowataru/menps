
#pragma once

#include "integral_constant.hpp"

namespace mgbase {

namespace detail {

template <typename> struct is_integral_helper             : false_type { };
template <> struct is_integral_helper<bool>               : true_type { };
template <> struct is_integral_helper<char>               : true_type { };
template <> struct is_integral_helper<signed char>        : true_type { };
template <> struct is_integral_helper<wchar_t>            : true_type { };
template <> struct is_integral_helper<short>              : true_type { };
template <> struct is_integral_helper<int>                : true_type { };
template <> struct is_integral_helper<long>               : true_type { };
template <> struct is_integral_helper<long long>          : true_type { };
template <> struct is_integral_helper<unsigned char>      : true_type { };
template <> struct is_integral_helper<unsigned short>     : true_type { };
template <> struct is_integral_helper<unsigned int>       : true_type { };
template <> struct is_integral_helper<unsigned long>      : true_type { };
template <> struct is_integral_helper<unsigned long long> : true_type { };

} // namespace detail

template <typename T>
struct is_integral
    : detail::is_integral_helper<typename remove_cv<T>::type> { };

} // namespace mgbase

