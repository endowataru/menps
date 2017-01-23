
#pragma once

#include <type_traits>

namespace mgbase {

#if 0
// Not available on GCC 4.4

template <typename T>
struct underlying_type
{
    typedef __underlying_type(T)    type;
};
#endif

#if 0
using std::underlying_type;
#endif

} // namespace mgbase

