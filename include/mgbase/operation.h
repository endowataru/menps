
#pragma once

#include <mgbase/lang.h>
#include <mgbase/bound_function.h>
#include <mgbase/atomic.h>

MGBASE_EXTERN_C_BEGIN

typedef enum mgbase_operation_code
{
    // Put no operation for fast switch-case
    MGBASE_OPERATION_NO_OPERATION
,   MGBASE_OPERATION_CALL_BOUND_FUNCTION

,   MGBASE_OPERATION_STORE_RELEASE_INT8
,   MGBASE_OPERATION_STORE_RELEASE_INT16
,   MGBASE_OPERATION_STORE_RELEASE_INT32
,   MGBASE_OPERATION_STORE_RELEASE_INT64

#define DEFINE_FETCH_OPERATION(op, OP)  \
,   MGBASE_OPERATION_FETCH_##OP##_INT8  \
,   MGBASE_OPERATION_FETCH_##OP##_INT16 \
,   MGBASE_OPERATION_FETCH_##OP##_INT32 \
,   MGBASE_OPERATION_FETCH_##OP##_INT64

MGBASE_FETCH_OP_LIST(DEFINE_FETCH_OPERATION)

#undef DEFINE_FETCH_OPERATION

}
mgbase_operation_code;

typedef struct mgbase_operation_operands {
    volatile void*      pointer;
    uint64_t            value;
}
mgbase_operation_operands;

typedef union mgbase_operation_argument {
    mgbase_operation_operands       operands;
    MGBASE_BOUND_FUNCTION(void ())  func;
}
mgbase_operation_argument;

typedef struct mgbase_operation
{
    mgbase_operation_code       code;
    mgbase_operation_argument   arg;
}
mgbase_operation;

MGBASE_EXTERN_C_END

