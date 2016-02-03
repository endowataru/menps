
#pragma once

#include <mgcom/common.h>
#include <mgbase/deferred.h>
#include <mgcom/am/roundtrip.h>
#include <mgbase/atomic.h>

MGBASE_EXTERN_C_BEGIN

// DEPRECATED: These will be replaced by try_*.

/// Control block for non-blocking contiguous read.
typedef struct mgcom_rma_remote_read_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_process_id_t          proc;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     local_addr;
    mgcom_index_t               size_in_bytes;
    
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // emulation on AM
    int                         tag;
    void*                       request;
}
mgcom_rma_remote_read_cb;

/// Control block for non-blocking contiguous write.
typedef struct mgcom_rma_remote_write_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_process_id_t          proc;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     local_addr;
    mgcom_index_t               size_in_bytes;
    
    // These members are required for emulation on AM
    mgcom_am_call_roundtrip_cb  cb_roundtrip;
    int                         tag;
    void*                       request;
}
mgcom_rma_remote_write_cb;

/// Control block for non-blocking remote atomic read.
typedef struct mgcom_rma_atomic_read_default_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_process_id_t          proc;
    mgcom_rma_local_address     local_addr;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     buf_addr;
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // emulation on AM
}
mgcom_rma_atomic_read_default_cb;

/// Control block for non-blocking remote atomic write.
typedef struct mgcom_rma_atomic_write_default_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_process_id_t          proc;
    mgcom_rma_local_address     local_addr;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     buf_addr;
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // emulation on AM
}
mgcom_rma_atomic_write_default_cb;


/// Control block for non-blocking local compare-and-swap.
typedef struct mgcom_rma_local_compare_and_swap_default_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_rma_local_address     target_addr;
    mgcom_rma_local_address     expected_addr;
    mgcom_rma_local_address     desired_addr;
    mgcom_rma_local_address     result_addr;
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // emulation on AM
}
mgcom_rma_local_compare_and_swap_default_cb;

/// Control block for non-blocking local fetch-and-add.
typedef struct mgcom_rma_local_fetch_and_add_default_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_rma_local_address     target_addr;
    mgcom_rma_local_address     value_addr;
    mgcom_rma_local_address     result_addr;
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // for emulation on AM
}
mgcom_rma_local_fetch_and_add_default_cb;


/// Control block for non-blocking remote compare-and-swap.
typedef struct mgcom_rma_remote_compare_and_swap_default_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_rma_remote_address    target_addr;
    mgcom_process_id_t          target_proc;
    mgcom_rma_local_address     expected_addr;
    mgcom_rma_local_address     desired_addr;
    mgcom_rma_local_address     result_addr;
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // for emulation on AM
}
mgcom_rma_remote_compare_and_swap_default_cb;

/// Control block for non-blocking fetch-and-add.
typedef struct mgcom_rma_remote_fetch_and_add_default_cb {
    MGBASE_CONTINUATION(void)   cont;
    mgbase_atomic_bool          finished;
    mgcom_rma_remote_address    target_addr;
    mgcom_process_id_t          target_proc;
    mgcom_rma_local_address     value_addr;
    mgcom_rma_local_address     result_addr;
    mgcom_am_call_roundtrip_cb  cb_roundtrip; // for emulation on AM
}
mgcom_rma_remote_fetch_and__default_cb;

MGBASE_EXTERN_C_END

