
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
struct remove_pointer { typedef T type; };

template <typename T>
struct remove_pointer<T*> { typedef T type; };

} // namespace mgbase

