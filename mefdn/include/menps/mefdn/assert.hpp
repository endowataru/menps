
#pragma once

#include <menps/mefdn/lang.hpp>

#undef NDEBUG

#include <cassert>

#if (!defined(MEFDN_ENABLE_ASSERT) && defined(MEFDN_DEBUG))
    #define MEFDN_ENABLE_ASSERT
#endif

#ifdef MEFDN_ENABLE_ASSERT
    #define MEFDN_ASSERT(x)    assert(x);
#else
    #define MEFDN_ASSERT(x)    if (false) { assert(x); }
#endif

