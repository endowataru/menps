
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_PROFILER_USE_GPERFTOOLS
    #include <google/profiler.h>
#endif

namespace mgbase {
namespace profiler {

inline void start()
{
    #ifdef MGBASE_PROFILER_USE_GPERFTOOLS
        // TODO: file name management
        ProfilerStart("gperf.prof");
    #endif
}
inline void stop()
{
    #ifdef MGBASE_PROFILER_USE_GPERFTOOLS
        ProfilerStop();
    #endif
}

} // namespace profiler
} // namespace mgbase

