
#pragma once

#include "lang.hpp"

#if (__cplusplus < 201103L)

namespace mgbase {

template <bool B, typename T = void>
struct enable_if;

template <typename T>
struct enable_if<true, T>
{
    typedef T   type;
};


template <typename T>
struct identity {
    typedef T   type;
};

}

#else

#include <type_traits>

namespace mgbase {

using std::enable_if;

}

#endif

