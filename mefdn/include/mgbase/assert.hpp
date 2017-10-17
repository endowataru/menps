
#pragma once

#include <mgbase/lang.hpp>

#undef NDEBUG

#include <cassert>

#if (!defined(MGBASE_ENABLE_ASSERT) && defined(MGBASE_DEBUG))
    #define MGBASE_ENABLE_ASSERT
#endif

#ifdef MGBASE_ENABLE_ASSERT
    #define MGBASE_ASSERT(x)    assert(x);
#else
    #define MGBASE_ASSERT(x)    if (false) { assert(x); }
#endif

