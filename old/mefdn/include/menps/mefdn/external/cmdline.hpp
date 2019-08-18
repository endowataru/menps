
#pragma once

#include <menps/mefdn/lang.hpp>

#ifdef MEFDN_COMPILER_CLANG
    #pragma clang system_header
#endif

#if defined(MEFDN_COMPILER_GCC) || defined (MEFDN_COMPILER_INTEL)
    #pragma GCC system_header
#endif

#if defined(MEFDN_COMPILER_INTEL)
    #pragma warning(push)
    #pragma warning(disable: 1418)
    #pragma warning(disable: 1944)
    #pragma warning(disable: 3280)
#endif

#include <cmdline.h>

#if defined(MEFDN_COMPILER_INTEL)
    #pragma warning(pop)
#endif


