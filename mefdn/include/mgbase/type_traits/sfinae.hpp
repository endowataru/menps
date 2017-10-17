
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

// From Boost.TypeTraits

namespace detail {

struct yes_type { char x;    };
struct no_type  { char x[2]; };

struct any_conversion
{
    template <typename T> any_conversion(const volatile T&);
    template <typename T> any_conversion(const T&);
    template <typename T> any_conversion(volatile T&);
    template <typename T> any_conversion(T&);
};

} // namespace detail

} // namespace mgbase

