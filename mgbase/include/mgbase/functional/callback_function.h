
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

typedef struct mgbase_untyped_callback_function {
    void (* func)();
    void* data;
}
mgbase_untyped_callback_function;

MGBASE_EXTERN_C_END

#ifdef MGBASE_CPLUSPLUS
    #define MGBASE_CALLBACK_FUNCTION(signature)  mgbase::callback_function<signature>
    
    #include "callback_function.hpp"
#else
    #define MGBASE_CALLBACK_FUNCTION(signature)  mgbase_untyped_callback_function
#endif

