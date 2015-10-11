
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <mgbase/lang.h>
#include <mgbase/control.h>


MGBASE_EXTERN_C_BEGIN

typedef uint64_t  mgcom_index_t;

typedef uint32_t  mgcom_process_id_t;

typedef uint64_t  mgcom_rma_address_offset_t;

/**
 * Region key.
 * Guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mgcom_rma_region_key {
    void*    pointer;
    
    // (ibv)   info = rkey
    // (fjmpi) info = memid
    // (mpi3)  info = win_id
    uint64_t info;
}
mgcom_rma_region_key;

/**
 * Local registered region.
 * NOT guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mgcom_rma_local_region {
    mgcom_rma_region_key key;
    
    // (ibv)   info = lkey
    // (fjmpi) info = laddr
    uint64_t info;
}
mgcom_rma_local_region;

/**
 * Remote registered region.
 * NOT guaranteed to be POD.
 * DO NOT access the members directly.
 */
typedef struct mgcom_rma_remote_region {
    mgcom_rma_region_key key;
    
    // (ibv)   info = (unused)
    // (fjmpi) info = raddr
    uint64_t info;
}
mgcom_rma_remote_region;

/**
 * Address of registered memory region located on the local process.
 */
typedef struct mgcom_rma_local_address {
    mgcom_rma_local_region            region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MGCOM_REGISTRATION_ALIGNMENT.
     */
    mgcom_rma_address_offset_t        offset;
}
mgcom_rma_local_address;

/**
 * Address of registered memory region located on a remote process.
 */
typedef struct mgcom_rma_remote_address {
    mgcom_rma_remote_region           region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MGCOM_REGISTRATION_ALIGNMENT.
     */
    mgcom_rma_address_offset_t        offset;
}
mgcom_rma_remote_address;


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


typedef enum mgcom_error_code {
    MGCOM_SUCCESS
,   MGCOM_FAILURE
}
mgcom_error_code_t;

/**
 * Initialize and start the communication.
 */
mgcom_error_code_t mgcom_initialize(int* argc, char*** argv) MGBASE_NOEXCEPT;

/**
 * Finalize the communication.
 */
mgcom_error_code_t mgcom_finalize(void) MGBASE_NOEXCEPT;

/**
 * Register a region located on the current process.
 */
mgcom_error_code_t mgcom_rma_register_region(
    void*                        local_pointer
,   mgcom_index_t                size_in_bytes
,   mgcom_rma_local_region*      result
) MGBASE_NOEXCEPT;


/**
 * Prepare a region located on a remote process.
 */
mgcom_error_code_t mgcom_rma_use_remote_region(
    mgcom_process_id_t           proc_id
,   mgcom_rma_region_key         key
,   mgcom_index_t                size_in_bytes
,   mgcom_rma_remote_region*     result
) MGBASE_NOEXCEPT;

/**
 * De-register the region located on the current process.
 */
mgcom_error_code_t mgcom_rma_deregister_region(
    mgcom_rma_local_region       region
,   void*                        local_pointer
,   mgcom_index_t                size_in_bytes
) MGBASE_NOEXCEPT;


/**
 * Registered buffer.
 */
typedef struct mgcom_rma_registered_buffer {
    mgcom_rma_local_address addr;
}
mgcom_rma_registered_buffer;

/**
 * Allocate a buffer from registered buffer pool.
 */
mgcom_error_code_t mgcom_rma_allocate(
    mgcom_index_t                   size_in_bytes
,   mgcom_rma_registered_buffer*    result
) MGBASE_NOEXCEPT;

/**
 * Deallocate a buffer allocated from registered buffer pool.
 */
mgcom_error_code_t mgcom_rma_deallocate(
    mgcom_rma_registered_buffer     buffer
) MGBASE_NOEXCEPT;


/// Control block for non-blocking contiguous read.
typedef struct mgcom_rma_remote_read_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     local_addr;
    mgcom_index_t               size_in_bytes;
}
mgcom_rma_remote_read_cb;

/// Control block for non-blocking contiguous write.
typedef struct mgcom_rma_remote_write_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     local_addr;
    mgcom_index_t               size_in_bytes;
}
mgcom_rma_remote_write_cb;



/// Control block for non-blocking strided write.
typedef struct mgcom_rma_write_strided_cb {
    mgbase_control_cb_common    common;
}
mgcom_rma_write_strided_cb;

/**
 * Non-blockng strided write.
 */
mgcom_error_code_t mgcom_rma_write_strided_nb(
    mgcom_rma_write_strided_cb* cb
,   mgcom_rma_local_address     local_addr
,   mgcom_index_t*              local_stride
,   mgcom_rma_remote_address    remote_addr
,   mgcom_index_t*              remote_stride
,   mgcom_index_t*              count
,   mgcom_index_t               stride_level
,   mgcom_process_id_t          dest_proc
) MGBASE_NOEXCEPT;


/// Control block for non-blocking strided read.
typedef struct mgcom_rma_read_strided_cb {
    mgbase_control_cb_common    common;
}
mgcom_rma_read_strided_cb;

/**
 * Non-blockng strided read.
 */
mgcom_error_code_t mgcom_rma_read_strided_nb(
    mgcom_rma_read_strided_cb*  cb
,   mgcom_rma_local_address     local_addr
,   mgcom_index_t*              local_stride
,   mgcom_rma_remote_address    remote_addr
,   mgcom_index_t*              remote_stride
,   mgcom_index_t*              count
,   mgcom_index_t               stride_level
,   mgcom_process_id_t          dest_proc
) MGBASE_NOEXCEPT;


/**
 * Default integer type for atomic operations.
 */
typedef mgbase_uint64_t     mgcom_rma_atomic_default_t;


/// Control block for non-blocking remote atomic read.
typedef struct mgcom_rma_atomic_read_default_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_local_address     local_addr;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     buf_addr;
}
mgcom_rma_atomic_read_default_cb;

/// Control block for non-blocking remote atomic write.
typedef struct mgcom_rma_atomic_write_default_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          proc;
    mgcom_rma_local_address     local_addr;
    mgcom_rma_remote_address    remote_addr;
    mgcom_rma_local_address     buf_addr;
}
mgcom_rma_atomic_write_default_cb;


/// Control block for non-blocking local compare-and-swap.
typedef struct mgcom_rma_local_compare_and_swap_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_local_address     target_addr;
    mgcom_rma_local_address     expected_addr;
    mgcom_rma_local_address     desired_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_local_compare_and_swap_default_cb;

/// Control block for non-blocking local fetch-and-add.
typedef struct mgcom_rma_local_fetch_and_add_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_local_address     target_addr;
    mgcom_rma_local_address     diff_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_local_fetch_and_add_default_cb;


/// Control block for non-blocking remote compare-and-swap.
typedef struct mgcom_rma_remote_compare_and_swap_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_remote_address    target_addr;
    mgcom_process_id_t          target_proc;
    mgcom_rma_local_address     expected_addr;
    mgcom_rma_local_address     desired_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_remote_compare_and_swap_default_cb;

/// Control block for non-blocking fetch-and-add.
typedef struct mgcom_rma_remote_fetch_and_add_default_cb {
    mgbase_control_cb_common    common;
    mgcom_rma_remote_address    target_addr;
    mgcom_process_id_t          target_proc;
    mgcom_rma_local_address     value_addr;
    mgcom_rma_local_address     result_addr;
}
mgcom_rma_remote_fetch_and__default_cb;


/**
 * Type of Unique IDs of Active Messages' handler.
 */
typedef mgcom_index_t  mgcom_am_handler_id_t;

typedef struct mgcom_callback_argument {
    mgcom_process_id_t source;
    const void*        data;
    mgcom_index_t      size;
}
mgcom_am_callback_parameters;

/**
 * Callback function of Active Messages' handler.
 */
typedef void (*mgcom_am_handler_callback_t)(const mgcom_am_callback_parameters*);

/**
 * Register a callback function as a Active Messages' handler.
 */
mgcom_error_code_t mgcom_am_register_handler(
    mgcom_am_handler_id_t
,   mgcom_am_handler_callback_t
);

#define MGCOM_AM_MAX_DATA_SIZE 1024
#define MGCOM_AM_HANDLE_SIZE   1024

typedef struct mgcom_am_message {
    mgcom_am_handler_id_t   id;
    mgcom_index_t           size;
    mgcom_index_t           ticket;
    uint8_t                 data[MGCOM_AM_MAX_DATA_SIZE]; // TODO
}
mgcom_am_message;

/// Control block for sending Active Messages.
typedef struct mgcom_am_send_cb {
    mgbase_control_cb_common    common;
    mgcom_process_id_t          dest_proc;
    mgcom_am_message            msg;
    uint8_t                     handle[MGCOM_AM_HANDLE_SIZE];
}
mgcom_am_send_cb;

/**
 * Invoke the callback function on the specified remote node.
 */
mgcom_error_code_t mgcom_am_send(
    mgcom_am_send_cb*     cb
,   mgcom_am_handler_id_t id
,   const void*           value
,   mgcom_index_t         size
,   mgcom_process_id_t    dest_proc
);


/**
 * Barrier (Collective)
 */
mgcom_error_code_t mgcom_barrier(void) MGBASE_NOEXCEPT;


mgcom_process_id_t mgcom_current_process_id(void) MGBASE_NOEXCEPT;

mgcom_index_t mgcom_number_of_processes(void) MGBASE_NOEXCEPT;

// TODO

#define MGCOM_REGISTRATION_ALIGNMENT  4
#define MGCOM_BUFFER_ALIGNMENT        4

#ifdef __cplusplus
    #define MGCOM_RMA_LOCAL_POINTER(type)   mgcom::typed_rma::local_pointer<type>
    #define MGCOM_RMA_REMOTE_POINTER(type)  mgcom::typed_rma::remote_pointer<type>
#else
    #define MGCOM_RMA_LOCAL_POINTER(type)   mgcom_rma_local_address
    #define MGCOM_RMA_REMOTE_POINTER(type)  mgcom_rma_remote_address
#endif

MGBASE_EXTERN_C_END

