
#pragma once

#ifdef __x86_64__

#include <cmpth/arch/x86_64_context_policy.hpp>

namespace cmpth {

template <typename P>
using context_policy = x86_64_context_policy<P>;

} // namespace cmpth

#else

// Unsupported architecture

#endif

