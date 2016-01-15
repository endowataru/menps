
#pragma once

#include <mgcom/common.h>

MGBASE_EXTERN_C_BEGIN

typedef struct mgcom_collective_barrier_cb {
    MGBASE_CONTINUATION(void)   cont;
    void*                       request;
}
mgcom_collective_barrier_cb;

typedef struct mgcom_collective_broadcast_cb {
    MGBASE_CONTINUATION(void)   cont;
    
    union {
        mgcom_collective_barrier_cb cb_barrier;
        void*                       request;
    }
    sync;
    
    mgcom_process_id_t          root;
    void*                       ptr;
    mgcom_index_t               number_of_bytes;
}
mgcom_collective_broadcast_cb;

typedef struct mgcom_collective_allgather_cb {
    MGBASE_CONTINUATION(void)   cont;
    
    union {
        mgcom_collective_barrier_cb cb_barrier;
        void*                       request;
    }
    sync;
    
    const void*                 src;
    void*                       dest;
    mgcom_index_t               number_of_bytes;
}
mgcom_collective_allgather_cb;

MGBASE_EXTERN_C_END

