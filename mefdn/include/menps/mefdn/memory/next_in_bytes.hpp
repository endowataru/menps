
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

inline void* next_in_bytes(void* const first, const mefdn::ptrdiff_t offset = 1)
{
    return static_cast<mefdn::uint8_t*>(first) + offset;
}
inline const void* next_in_bytes(const void* const first, const mefdn::ptrdiff_t offset = 1)
{
    return static_cast<const mefdn::uint8_t*>(first) + offset;
}

} // namespace mefdn
} // namespace menps

