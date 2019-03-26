
#pragma once

#include <mgbase/callback_function.hpp>

MGBASE_EXTERN_C_BEGIN

typedef struct mgbase_untyped_callback_function
    mgbase_untyped_resumable;

#ifdef MGBASE_CPLUSPLUS
    #define MGBASE_RESUMABLE(T) mgbase::resumable
    
    #include "resumable.hpp"
#else
    #define MGBASE_RESUMABLE(T) mgbase_untyped_resumable
#endif

MGBASE_EXTERN_C_END

