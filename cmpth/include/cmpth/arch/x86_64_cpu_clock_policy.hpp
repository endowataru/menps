
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct x86_64_cpu_clock_policy
{
    using clock_type = fdn::int64_t;
    
    static clock_type get_clock() noexcept
    {
        uint32_t high, low;
        asm volatile ("lfence\nrdtsc" : "=a"(low), "=d"(high));
        return (static_cast<clock_type>(high) << 32) | low;
    }
};

} // namespace cmpth

