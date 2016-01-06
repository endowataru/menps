
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

typedef void (*mgbase_untyped_binded_function_func_ptr_t)();

typedef struct mgbase_untyped_binded_function {
    mgbase_untyped_binded_function_func_ptr_t func;
    void* arg1;
}
mgbase_untyped_binded_function;

#ifdef MGBASE_CPLUSPLUS
    #define MGBASE_BINDED_FUNCTION(signature) mgbase::binded_function<signature>
#else
    #define MGBASE_BINDED_FUNCTION(signature) mgbase_untyped_binded_function
#endif

MGBASE_EXTERN_C_END

