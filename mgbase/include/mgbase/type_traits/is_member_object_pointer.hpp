
#pragma once

#include "is_member_pointer.hpp"
#include "is_member_function_pointer.hpp"

namespace mgbase {

template <typename T>
struct is_member_object_pointer
    : bool_constant<(
        mgbase::is_member_pointer<T>::value &&
        ! mgbase::is_member_function_pointer<T>::value
    )> {};

} // namespace mgbase

