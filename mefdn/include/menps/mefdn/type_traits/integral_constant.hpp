
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

template <typename T, T v>
struct integral_constant
{
    static const T value = v;
    
    typedef T                   value_type;
    typedef integral_constant   type;
    
    operator value_type() const noexcept { return value; }
    value_type operator() () const noexcept { return value; }
};

template <bool B>
struct bool_constant : integral_constant<bool, B> { };

typedef bool_constant<true>   true_type;
typedef bool_constant<false>  false_type;

} // namespace mefdn
} // namespace menps

