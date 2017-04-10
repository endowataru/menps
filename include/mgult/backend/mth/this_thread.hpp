
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

} // namespace this_thread

} // namespace mth
} // namespace backend
} // namespace mgult


