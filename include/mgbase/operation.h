
#pragma once

#include <mgbase/lang.h>

MGBASE_EXTERN_C_BEGIN

typedef enum mgbase_operation_code {
    MGBASE_OPERATION_ASSIGN_INT8
,   MGBASE_OPERATION_ASSIGN_INT16
,   MGBASE_OPERATION_ASSIGN_INT32
,   MGBASE_OPERATION_ASSIGN_INT64
,   MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT32
,   MGBASE_OPERATION_ATOMIC_FETCH_ADD_INT64
    // Put no operation for fast switch-case
,   MGBASE_OPERATION_NO_OPERATION
}
mgbase_operation_code;

typedef struct mgbase_operation {
    mgbase_operation_code   code;
    void*                   pointer;
    uint64_t                value;
}
mgbase_operation;

MGBASE_EXTERN_C_END

