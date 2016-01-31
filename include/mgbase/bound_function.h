
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

typedef void (*mgbase_untyped_bound_function_func_ptr_t)();

typedef struct mgbase_untyped_bound_function {
    mgbase_untyped_bound_function_func_ptr_t func;
    void* arg1;
}
mgbase_untyped_bound_function;

MGBASE_EXTERN_C_END

#ifdef MGBASE_CPLUSPLUS
    #define MGBASE_BOUND_FUNCTION(signature) mgbase::bound_function<signature>
    #include <mgbase/bound_function.hpp>
#else
    #define MGBASE_BOUND_FUNCTION(signature) mgbase_untyped_bound_function
#endif


