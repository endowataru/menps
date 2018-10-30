
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

// offsetof() macro in <stddef.h>

template <typename T, typename M>
inline constexpr mefdn::ptrdiff_t get_offset_of(M T::* const mem)
{
    return reinterpret_cast<mefdn::ptrdiff_t>(
        &(static_cast<T*>(nullptr)->*mem)
    );
}

// container_of() macro in Linux kernel

template <typename T, typename M>
inline constexpr T* get_container_of(M* const p, M T::* const mem)
{
    return reinterpret_cast<T*>(
        reinterpret_cast<mefdn::uintptr_t>(p) - get_offset_of(mem)
    );
}

} // namespace mefdn
} // namespace menps

