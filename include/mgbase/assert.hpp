
#pragma once

#include <cassert>

#ifdef MGBASE_DEBUG
    #define MGBASE_ASSERT(x)    assert(x);
#else
    #define MGBASE_ASSERT(x)
#endif

