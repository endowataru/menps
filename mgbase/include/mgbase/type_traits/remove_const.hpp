
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T> struct remove_const          { typedef T type; };
template <typename T> struct remove_const<const T> { typedef T type; };

} // namespace mgbase

