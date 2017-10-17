
#pragma once

#include <mgbase/atomic.h>

MGBASE_EXTERN_C_BEGIN

typedef struct mgbase_wait_flag
{
    mgbase_atomic_bool  flag;
}
mgbase_wait_flag;

MGBASE_EXTERN_C_END

