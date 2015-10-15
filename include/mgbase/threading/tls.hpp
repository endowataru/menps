
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_CPP11_SUPPORTED
    #define MGBASE_THREAD_LOCAL     thread_local
#else
    #define MGBASE_THREAD_LOCAL     __thread
#endif

