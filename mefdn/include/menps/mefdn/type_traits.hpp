
#pragma once

#include <menps/mefdn/lang.hpp>
#include <type_traits>

namespace menps {
namespace mefdn {

using std::is_reference;
using std::conditional;
using std::is_const;
using std::is_same;
using std::is_void;
using std::is_signed;
using std::is_trivial;

template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using remove_const_t = typename std::remove_const<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

using std::declval;

} // namespace mefdn
} // namespace menps

