
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

template <typename T, T Value>
struct nontype
{
    // TODO: function pointers cannot be "static const" members
    //static const T value = Value;
    
    T operator() () const volatile {
        return Value;
    }
};

#define MEFDN_NONTYPE(v)    (::mefdn::nontype<decltype(v), v>{})

} // namespace mefdn
} // namespace menps

