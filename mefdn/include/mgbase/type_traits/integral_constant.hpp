
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, T v>
struct integral_constant
{
    static const T value = v;
    
    typedef T                   value_type;
    typedef integral_constant   type;
    
    operator value_type() const MGBASE_NOEXCEPT { return value; }
    value_type operator() () const MGBASE_NOEXCEPT { return value; }
};

template <bool B>
struct bool_constant : integral_constant<bool, B> { };

typedef bool_constant<true>   true_type;
typedef bool_constant<false>  false_type;

} // namespace mgbase

