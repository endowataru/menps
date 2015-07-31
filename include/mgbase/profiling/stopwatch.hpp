
#pragma once

#include <mgbase/stdint.hpp>

namespace mgbase {

namespace detail {

typedef int64_t  cpu_clock_t;

inline cpu_clock_t rdtsc() noexcept
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
    
    #if MGBASE_CPP11_SUPPORTED
    stopwatch(const stopwatch&) = default;
    stopwatch& operator = (const stopwatch&) noexcept = default;
    #endif
    
    void start() noexcept { began_ = current(); }
    
    detail::cpu_clock_t elapsed() const noexcept { return current() - began_; }
    
private:
    static detail::cpu_clock_t current() noexcept { return detail::rdtsc(); }
    
    detail::cpu_clock_t began_;
};

}

