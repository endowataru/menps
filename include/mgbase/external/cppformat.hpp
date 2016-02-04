
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

#include <cppformat/format.h>

