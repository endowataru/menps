
#pragma once

#include "continuation.hpp"

namespace mgbase {

/**
 * Default function for locating the reference of the next continuation.
 *
 * You can customize this by overloading in the namespace of CB.
 * (It will be searched using ADL.)
 */
template <typename T, typename CB>
inline continuation<T>& get_next_continuation(CB& cb)
{
    // cont is the default member name.
    return cb.cont;
}

} // namespace mgbase

