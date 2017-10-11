
#pragma once

#include <mgbase/lang.hpp>

#ifdef MGBASE_ARCH_SPARC64_IXFX
    #include "arch/sparc64_ixfx.hpp"
#elif defined(MGBASE_ARCH_INTEL)
    #include "arch/x86_64.hpp"
#else
    #include "arch/gcc_sync.hpp"
#endif

