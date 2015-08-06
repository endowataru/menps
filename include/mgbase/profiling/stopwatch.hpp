
#pragma once

#include <mgbase/stdint.hpp>

namespace mgbase {

namespace detail {

typedef int64_t  cpu_clock_t;

inline cpu_clock_t rdtsc() MGBASE_NOEXCEPT
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

}

class stopwatch {
public:
    stopwatch() MGBASE_NOEXCEPT { start(); }
    
    #ifdef MGBASE_CPP11_SUPPORTED
    stopwatch(const stopwatch&) = default;
    stopwatch& operator = (const stopwatch&) noexcept = default;
    #endif
    
    void start() MGBASE_NOEXCEPT { began_ = current(); }
    
    detail::cpu_clock_t elapsed() const MGBASE_NOEXCEPT { return current() - began_; }
    
private:
    static detail::cpu_clock_t current() MGBASE_NOEXCEPT { return detail::rdtsc(); }
    
    detail::cpu_clock_t began_;
};

}

