
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

template <bool B, typename T = void>
struct enable_if;

template <typename T>
struct enable_if<true, T>
{
    typedef T   type;
};

} // namespace mgbase

