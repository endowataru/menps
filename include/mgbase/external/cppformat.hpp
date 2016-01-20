
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_COMPILER_FUJITSU

#include <math.h>

// Workaround for Fujitsu compiler

#undef isnan
#undef isinf
#undef signbit

#endif

#include <cppformat/format.h>

