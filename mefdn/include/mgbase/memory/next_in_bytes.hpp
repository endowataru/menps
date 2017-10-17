
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

inline void* next_in_bytes(void* const first, const mgbase::ptrdiff_t offset = 1)
{
    return static_cast<mgbase::uint8_t*>(first) + offset;
}
inline const void* next_in_bytes(const void* const first, const mgbase::ptrdiff_t offset = 1)
{
    return static_cast<const mgbase::uint8_t*>(first) + offset;
}

} // namespace mgbase

