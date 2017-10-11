
#pragma once

#include <mgbase/lang.hpp>

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

#include <gtest/gtest.h>

#if defined(MGBASE_COMPILER_INTEL)
    #pragma warning(pop)
#endif

