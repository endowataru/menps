
#pragma once

#include <mgbase/type_traits/add_rvalue_reference.hpp>

namespace mgbase {

template <typename T>
typename add_rvalue_reference<T>::type declval() MGBASE_NOEXCEPT;

} // namespace mgbase

