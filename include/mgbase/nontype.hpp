
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, T Value>
struct nontype
{
    static const T value = Value;
    
    T operator() () const volatile {
        return value;
    }
};

#define MGBASE_NONTYPE(v)   (::mgbase::nontype<decltype(v), (v)>{})

} // namespace mgbase

