
#pragma once

#include "clock.hpp"

namespace menps {
namespace mefdn {

class stopwatch {
public:
    stopwatch() noexcept { start(); }
    
    stopwatch(const stopwatch&) noexcept = default;
    stopwatch& operator = (const stopwatch&) noexcept = default;
    
    void start() noexcept { began_ = current(); }
    
    cpu_clock_t elapsed() const noexcept { return current() - began_; }
    
private:
    static cpu_clock_t current() noexcept {
        return get_cpu_clock();
    }
    
    cpu_clock_t began_;
};

} // namespace mefdn
} // namespace menps

