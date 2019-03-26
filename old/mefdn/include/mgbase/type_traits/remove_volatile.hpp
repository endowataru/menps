
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T> struct remove_volatile             { typedef T type; };
template <typename T> struct remove_volatile<volatile T> { typedef T type; };

} // namespace mgbase

