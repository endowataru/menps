
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <typename T, T Value>
struct nontype
{
    // TODO: function pointers cannot be "static const" members
    //static const T value = Value;
    
    T operator() () const volatile {
        return Value;
    }
};

#define MGBASE_NONTYPE(v)   (::mgbase::nontype<decltype(v), v>{})

} // namespace mgbase

