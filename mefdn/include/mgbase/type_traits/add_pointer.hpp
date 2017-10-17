
#pragma once

#include "remove_reference.hpp"

namespace mgbase {

template <typename T>
struct add_pointer
{
    typedef typename mgbase::remove_reference<T>::type* type;
};

} // namespace mgbase

