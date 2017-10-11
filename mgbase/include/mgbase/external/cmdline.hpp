
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_COMPILER_CLANG
    #pragma clang system_header
#endif

#if defined(MGBASE_COMPILER_GCC) || defined (MGBASE_COMPILER_INTEL)
    #pragma GCC system_header
#endif

#if defined(MGBASE_COMPILER_INTEL)
    #pragma warning(push)
    #pragma warning(disable: 1418)
    #pragma warning(disable: 1944)
    #pragma warning(disable: 3280)
#endif

#include <cmdline.h>

#if defined(MGBASE_COMPILER_INTEL)
    #pragma warning(pop)
#endif


