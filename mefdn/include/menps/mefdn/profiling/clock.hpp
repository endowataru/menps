
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

typedef mefdn::int64_t cpu_clock_t;

MEFDN_ALWAYS_INLINE cpu_clock_t get_cpu_clock() noexcept
{
#if (defined __i386__) || (defined __x86_64__)
    uint32_t high, low;
    asm volatile ("lfence\nrdtsc" : "=a"(low),"=d"(high));
    return (static_cast<cpu_clock_t>(high) << 32) | low;

#elif (defined __sparc__) && (defined __arch64__)
    uint64_t tick;
    asm volatile ("rd %%tick, %0" : "=r" (tick));
    return static_cast<cpu_clock_t>(tick);

#else
    #warning "rdtsc() is not implemented."
    return 0;
#endif
}

} // namespace mefdn
} // namespace menps

