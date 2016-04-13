
#pragma once

#include <mgcom/rma/pointer.h>

MGBASE_EXTERN_C_BEGIN

typedef struct mgcom_rma_untyped_remote_read_params {
    mgcom_process_id_t      src_proc;
    mgcom_remote_address    src_raddr;
    mgcom_local_address     dest_laddr;
    mgcom_index_t           size_in_bytes;
}
mgcom_rma_untyped_remote_read_params;

typedef struct mgcom_rma_untyped_remote_write_params {
    mgcom_process_id_t      dest_proc;
    mgcom_remote_address    dest_raddr;
    mgcom_local_address     src_laddr;
    mgcom_index_t           size_in_bytes;
}
mgcom_rma_untyped_remote_write_params;

typedef struct mgcom_rma_remote_atomic_read_default_params {
    mgcom_process_id_t      target_proc;
    MGCOM_RMA_REMOTE_POINTER
    (mgcom_rma_atomic_default_t)    src_rptr;
    MGCOM_RMA_REMOTE_POINTER()  dest_lptr;
    mgcom_rma_local_address     buf_lptr;
}
mgcom_rma_remote_atomic_read_default_params;

MGBASE_EXTERN_C_END

