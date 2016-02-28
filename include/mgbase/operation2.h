
#pragma once

#include <mgbase/callback_function.h>

MGBASE_EXTERN_C_BEGIN

typedef struct mgbase_operation
{
    MGBASE_CALLBACK_FUNCTION
    (void (const mgbase_operation&))    func;
    
    mgbase_uint64_t                     value;
}
mgbase_operation;

MGBASE_EXTERN_C_END

