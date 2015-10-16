
#pragma once

#include <mgcom.h>

MGBASE_EXTERN_C_BEGIN

typedef enum mgcom_local_operation {
    MGCOM_LOCAL_NO_OPERATION
,   MGCOM_LOCAL_ASSIGN_INT8
,   MGCOM_LOCAL_ASSIGN_INT16
,   MGCOM_LOCAL_ASSIGN_INT32
,   MGCOM_LOCAL_ASSIGN_INT64
,   MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32
,   MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64
}
mgcom_local_operation;

typedef struct mgcom_notifier {
    mgcom_local_operation   operation;
    void*                   pointer;
    uint64_t                value;
}
mgcom_local_notifier;

typedef enum mgcom_remote_operation {
    MGCOM_REMOTE_CAS_INT64
,   MGCOM_REMOTE_FAA_INT64
}
mgcom_remote_operation;

MGBASE_EXTERN_C_END

