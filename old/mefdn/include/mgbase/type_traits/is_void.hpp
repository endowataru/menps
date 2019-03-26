
#pragma once

#include "is_same.hpp"
#include "remove_cv.hpp"

namespace mgbase {

template <typename T >
struct is_void : is_same<void, typename remove_cv<T>::type> { };

} // namespace mgbase

