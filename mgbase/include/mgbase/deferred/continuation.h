
#pragma once

#include <mgbase/callback_function.h>

MGBASE_EXTERN_C_BEGIN

typedef mgbase_untyped_callback_function
    mgbase_untyped_continuation;

MGBASE_EXTERN_C_END

#ifdef MGBASE_CPLUSPLUS
    #define MGBASE_CONTINUATION(T)  mgbase::continuation<T>
    
    #include "continuation.hpp"
#else
    #define MGBASE_CONTINUATION(T)  mgbase_untyped_continuation
#endif

