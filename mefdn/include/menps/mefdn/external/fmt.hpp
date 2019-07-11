
#pragma once

#include <menps/mefdn/lang.hpp>

#ifdef MEFDN_COMPILER_CLANG
    #pragma clang system_header
#endif

#if defined(MEFDN_COMPILER_GCC)
    #pragma GCC system_header
#endif

#if defined(MEFDN_COMPILER_INTEL)
    #pragma warning(push)
    #pragma warning(disable: 1418)
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#if defined(MEFDN_COMPILER_INTEL)
    #pragma warning(pop)
#endif

