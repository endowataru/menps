
#pragma once

#include <menps/mefdn/lang.hpp>
#include <sys/time.h>

namespace menps {
namespace mefdn {

inline double get_current_sec()
{
    timeval t;
    
    gettimeofday(&t, NULL);
    
    return t.tv_sec + t.tv_usec * 1e-6;
}

} // namespace mefdn
} // namespace menps

