
#pragma once

#include <menps/mefdn/lang.hpp>

#ifdef MEFDN_PROFILER_USE_GPERFTOOLS
    #include <google/profiler.h>
#endif

namespace menps {
namespace mefdn {
namespace profiler {

inline void start()
{
    #ifdef MEFDN_PROFILER_USE_GPERFTOOLS
        // TODO: file name management
        ProfilerStart("gperf.prof");
    #endif
}
inline void stop()
{
    #ifdef MEFDN_PROFILER_USE_GPERFTOOLS
        ProfilerStop();
    #endif
}

} // namespace profiler
} // namespace mefdn
} // namespace menps

