
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T>
struct add_lvalue_reference {
    typedef T&  type;
};
template <typename T>
struct add_lvalue_reference<T&> {
    typedef T&  type;
};
template <typename T>
struct add_lvalue_reference<T&&> {
    typedef T&  type;
};

} // namespace mgbase

