
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_COMPILER_FUJITSU

#include <math.h>

// Workaround for Fujitsu compiler

#undef isnan
#undef isinf
#undef signbit

#endif

#ifdef MGBASE_COMPILER_CLANG
    #pragma clang system_header
#endif

#if defined(MGBASE_COMPILER_GCC)
    #pragma GCC system_header
#endif

#if defined(MGBASE_COMPILER_INTEL)
    #pragma warning(push)
    #pragma warning(disable: 1418)
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>

#if defined(MGBASE_COMPILER_INTEL)
    #pragma warning(pop)
#endif


