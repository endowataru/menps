
#pragma once

#ifdef __x86_64__

#include <cmpth/arch/x86_64_cpu_clock_policy.hpp>

namespace cmpth {

template <typename P>
using cpu_clock_policy = x86_64_cpu_clock_policy<P>;

} // namespace cmpth

#else

// Unsupported architecture

#endif
