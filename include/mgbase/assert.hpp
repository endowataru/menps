
#pragma once

#undef NDEBUG

#include <cassert>

#ifdef MGBASE_DISABLE_ASSERT
    #define MGBASE_ASSERT(x)
#else
    #define MGBASE_ASSERT(x)    assert(x);
#endif

