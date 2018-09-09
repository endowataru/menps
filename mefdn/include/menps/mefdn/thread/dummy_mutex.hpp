
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

class dummy_mutex
{
public:
    dummy_mutex() = default;
    
    dummy_mutex(const dummy_mutex&) = delete;
    dummy_mutex& operator = (const dummy_mutex&) = delete;
    
    void lock() { }
    
    bool try_lock() { return true; }
    
    void unlock() { }
};

} // namespace mefdn
} // namespace menps

