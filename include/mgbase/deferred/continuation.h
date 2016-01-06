
#pragma once

#include <mgbase/binded_function.h>

MGBASE_EXTERN_C_BEGIN

typedef mgbase_untyped_binded_function
    mgbase_untyped_continuation;

#ifdef MGBASE_CPLUSPLUS
    #define MGBASE_CONTINUATION(T)  mgbase::continuation<T>
#else
    #define MGBASE_CONTINUATION(T)  mgbase_untyped_continuation
#endif

MGBASE_EXTERN_C_END

