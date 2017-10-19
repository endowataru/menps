
// May be included multiple times

#include <menps/mefdn/lang.hpp>

#ifdef MEFDN_COMPILER_CLANG
    #pragma clang system_header
#endif

#if defined(MEFDN_COMPILER_GCC) || defined (MEFDN_COMPILER_INTEL)
    #pragma GCC system_header
#endif

#include MEFDN_SYSTEM_HEADER

#undef MEFDN_SYSTEM_HEADER

