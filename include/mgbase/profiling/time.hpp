
#pragma once

#include <mgbase/lang.hpp>
#include <sys/time.h>

namespace mgbase {

inline double get_current_sec()
{
    timeval t;
    
    gettimeofday(&t, NULL);
    
    return t.tv_sec + t.tv_usec * 1e-9;
}

} // namespace mgbase

