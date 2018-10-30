
#pragma once

#include <menps/mefdn/lang.hpp>
#include <type_traits>

namespace menps {
namespace mefdn {

using std::is_convertible;
using std::remove_const;
using std::remove_extent;
using std::remove_cv;
using std::result_of;
using std::decay;
using std::is_reference;
using std::conditional;
using std::remove_reference;
using std::is_integral;
using std::is_const;
using std::enable_if;
using std::is_compound;
using std::is_array;
using std::is_pointer;
using std::is_enum;
using std::is_same;
using std::is_void;
using std::remove_pointer;
using std::add_lvalue_reference;
using std::is_signed;

template <typename T>
using decay_t = typename decay<T>::type;

template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

template <typename T>
using remove_const_t = typename remove_const<T>::type;

template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

template <typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

template <bool Condition, class T = void>
using enable_if_t = typename enable_if<Condition,T>::type;

// "identity" metafunction
// Proposed in P0887R1

template <typename T>
struct type_identity { using type = T; };

template <typename T>
using type_identity_t = typename type_identity<T>::type;

} // namespace mefdn
} // namespace menps

