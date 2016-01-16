
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_COMPILER_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wc++11-extensions"
#endif

#ifdef MGBASE_COMPILER_GCC
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
    #pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#endif

#include <gtest/gtest.h>

#ifdef MGBASE_COMPILER_CLANG
    #pragma clang diagnostic pop
#endif

#ifdef MGBASE_COMPILER_GCC
    #pragma GCC diagnostic pop
#endif


