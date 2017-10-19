
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

inline mefdn::ptrdiff_t distance_in_bytes(void* const first, void* const last)
{
    const auto f = reinterpret_cast<mefdn::intptr_t>(first);
    const auto l = reinterpret_cast<mefdn::intptr_t>(last);
    
    return static_cast<mefdn::ptrdiff_t>(l - f);
}

} // namespace mefdn
} // namespace menps

