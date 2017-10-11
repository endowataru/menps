
// May be included multiple times

#include <mgbase/lang.hpp>

#ifdef MGBASE_COMPILER_CLANG
    #pragma clang system_header
#endif

#if defined(MGBASE_COMPILER_GCC) || defined (MGBASE_COMPILER_INTEL)
    #pragma GCC system_header
#endif

#include MGBASE_SYSTEM_HEADER

#undef MGBASE_SYSTEM_HEADER

