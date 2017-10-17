
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
struct remove_extent { typedef T type; };
 
template <typename T>
struct remove_extent<T []> { typedef T type; };
 
template <typename T, mgbase::size_t N>
struct remove_extent<T [N]> { typedef T type;};

} // namespace mgbase

