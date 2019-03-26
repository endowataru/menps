
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T> struct is_lvalue_reference     : false_type {};
template <typename T> struct is_lvalue_reference<T&> : true_type {};

} // namespace mgbase

