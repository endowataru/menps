
#pragma once

#include "clock.hpp"

namespace mgbase {

class stopwatch {
public:
    stopwatch() MGBASE_NOEXCEPT { start(); }
    
    #ifdef MGBASE_CXX11_SUPPORTED
    stopwatch(const stopwatch&) = default;
    stopwatch& operator = (const stopwatch&) noexcept = default;
    #endif
    
    void start() MGBASE_NOEXCEPT { began_ = current(); }
    
    cpu_clock_t elapsed() const MGBASE_NOEXCEPT { return current() - began_; }
    
private:
    static cpu_clock_t current() MGBASE_NOEXCEPT {
        return get_cpu_clock();
    }
    
    cpu_clock_t began_;
};

} // namespace mgbase

