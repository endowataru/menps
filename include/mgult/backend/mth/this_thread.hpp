
#pragma once

#include "mth.hpp"

namespace mgult {
namespace backend {
namespace mth {

namespace this_thread {

inline void yield()
{
    myth_yield();
}

// non-standard function
inline void detach()
{
    myth_detach(myth_self());
}

} // namespace this_thread

} // namespace mth
} // namespace backend
} // namespace mgult


