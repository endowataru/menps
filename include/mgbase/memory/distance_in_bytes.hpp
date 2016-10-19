
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

inline mgbase::ptrdiff_t distance_in_bytes(void* const first, void* const last)
{
    const auto f = reinterpret_cast<mgbase::intptr_t>(first);
    const auto l = reinterpret_cast<mgbase::intptr_t>(last);
    
    return static_cast<mgbase::ptrdiff_t>(l - f);
}

} // namespace mgbase

