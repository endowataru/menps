
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

#define MEFDN_NONTYPE(...) \
    (::menps::mefdn::nontype<decltype(__VA_ARGS__), __VA_ARGS__>{})

// old macro used in templates
#define MEFDN_NONTYPE_TEMPLATE(...) \
    (::menps::mefdn::nontype<decltype(__VA_ARGS__), __VA_ARGS__>{})

} // namespace mefdn
} // namespace menps

