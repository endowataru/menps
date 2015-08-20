
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <mgbase/lang.h>
#include <mgbase/event/async_request.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t  mgcom_index_t;

typedef uint32_t  mgcom_process_id_t;

typedef uint64_t  mgcom_address_offset_t;

typedef struct mgcom_region_key_tag {
    uint64_t info[2];
}
mgcom_region_key;

typedef struct mgcom_local_region_tag {
    uint64_t local;
    mgcom_region_key key;
}
mgcom_local_region;

typedef struct mgcom_remote_region_tag {
    uint64_t info[2];
}
mgcom_remote_region;

/**
 * Address of registered memory region located on the local process.
 */
typedef struct mgcom_local_address_tag {
    mgcom_local_region            region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MGCOM_REGISTRATION_ALIGNMENT.
     */
    mgcom_address_offset_t        offset;
}
mgcom_local_address;

/**
 * Address of registered memory region located on a remote process.
 */
typedef struct mgcom_remote_region_address_tag {
    mgcom_remote_region           region;
    
    /**
     * Offset from the base position.
     * Must be aligned by MGCOM_REGISTRATION_ALIGNMENT.
     */
    mgcom_address_offset_t        offset;
}
mgcom_remote_address;


typedef enum mgcom_local_operation_tag {
    MGCOM_LOCAL_NO_OPERATION
,   MGCOM_LOCAL_ASSIGN_INT8
,   MGCOM_LOCAL_ASSIGN_INT64
,   MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32
,   MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64
}
mgcom_local_operation;

typedef struct mgcom_notifier_tag {
    mgcom_local_operation   operation;
    void*                   pointer;
    uint64_t                value;
}
mgcom_local_notifier;


typedef enum mgcom_remote_operation_tag {
    MGCOM_REMOTE_CAS_INT64
,   MGCOM_REMOTE_FAA_INT64
}
mgcom_remote_operation;


typedef enum mgcom_error_tag {
    MGCOM_SUCCESS
,   MGCOM_FAILURE
}
mgcom_error_t;

/**
 * Initialize and start the communication.
 */
mgcom_error_t mgcom_initialize(int* argc, char*** argv) MGBASE_NOEXCEPT;

/**
 * Finalize the communication.
 */
mgcom_error_t mgcom_finalize(void) MGBASE_NOEXCEPT;

/**
 * Register a region located on the current process.
 */
mgcom_error_t mgcom_register_region(
    void*                          local_pointer
,   mgcom_index_t                  size_in_bytes
,   mgcom_local_region*            result
) MGBASE_NOEXCEPT;


/**
 * Prepare a region located on a remote process.
 */
mgcom_error_t mgcom_use_remote_region(
    mgcom_process_id_t             proc_id
,   mgcom_region_key               key
,   mgcom_index_t                  size_in_bytes
,   mgcom_remote_region*           result
) MGBASE_NOEXCEPT;

/**
 * De-register the region located on the current process.
 */
mgcom_error_t mgcom_deregister_region(
    mgcom_local_region             region
,   void*                          local_pointer
,   mgcom_index_t                  size_in_bytes
) MGBASE_NOEXCEPT;


/**
 * Low-level function of contiguous write.
 */
mgcom_error_t mgcom_try_write_async(
    mgcom_local_address            local_addr
,   mgcom_remote_address           remote_addr
,   mgcom_index_t                  size_in_bytes
,   mgcom_process_id_t             dest_proc
,   mgcom_local_notifier           on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

/**
 * Low-level function of contiguous read.
 */
mgcom_error_t mgcom_try_read_async(
    mgcom_local_address            local_addr
,   mgcom_remote_address           remote_addr
,   mgcom_index_t                  size_in_bytes
,   mgcom_process_id_t             dest_proc
,   mgcom_local_notifier           on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;


/// Control block for non-blocking contiguous write.
typedef struct mgcom_write_cb_tag {
    mgbase_async_request request;
}
mgcom_write_cb;

/**
 * Non-blocking contiguous write.
 */
mgcom_error_t mgcom_write_async(
    mgcom_write_cb*           cb
,   mgcom_local_address       local_addr
,   mgcom_remote_address      remote_addr
,   mgcom_index_t             size_in_bytes
,   mgcom_process_id_t        dest_proc
) MGBASE_NOEXCEPT;


/// Control block for non-blocking contiguous read.
typedef struct mgcom_read_cb_tag {
    mgbase_async_request request;
}
mgcom_read_cb;

/**
 * Non-blocking contiguous read.
 */
mgcom_error_t mgcom_read_async(
    mgcom_read_cb*           cb
,   mgcom_local_address      local_addr
,   mgcom_remote_address     remote_addr
,   mgcom_index_t            size_in_bytes
,   mgcom_process_id_t       dest_proc
) MGBASE_NOEXCEPT;



/// Control block for asynchronous strided write.
typedef struct mgcom_write_strided_cb_tag {
    mgbase_async_request request;
}
mgcom_write_strided_cb;

/**
 * Non-blockng strided write.
 */
mgcom_error_t mgcom_write_strided_async(
    mgcom_write_strided_cb* cb
,   mgcom_local_address     local_addr
,   mgcom_index_t*          local_stride
,   mgcom_remote_address    remote_addr
,   mgcom_index_t*          remote_stride
,   mgcom_index_t*          count
,   mgcom_index_t           stride_level
,   mgcom_process_id_t      dest_proc
) MGBASE_NOEXCEPT;


/// Control block for asynchronous strided read.
typedef struct mgcom_read_strided_cb_tag {
    mgbase_async_request request;
}
mgcom_read_strided_cb;

/**
 * Non-blockng strided read.
 */
mgcom_error_t mgcom_read_strided_async(
    mgcom_read_strided_cb* cb
,   mgcom_local_address    local_addr
,   mgcom_index_t*         local_stride
,   mgcom_remote_address   remote_addr
,   mgcom_index_t*         remote_stride
,   mgcom_index_t*         count
,   mgcom_index_t          stride_level
,   mgcom_process_id_t     dest_proc
) MGBASE_NOEXCEPT;


/// Control block for non-blocking remote atomic operation.
typedef struct mgcom_rmw_cb_tag {
    mgbase_async_request request;
}
mgcom_rmw_cb;

/**
 * Non-blocking remote atomic operation.
 */
mgcom_error_t mgcom_rmw_async(
    mgcom_rmw_cb*           cb
,   mgcom_remote_operation  operation
,   void*                   local_expected
,   mgcom_local_address     local_addr
,   mgcom_remote_address    remote_addr
,   mgcom_process_id_t      dest_proc
) MGBASE_NOEXCEPT;


/**
 * Type of Unique IDs of Active Messages' handler.
 */
typedef mgcom_index_t  mgcom_am_handler_id_t;

/**
 * Callback function of Active Messages' handler.
 */
typedef void (*mgcom_am_handler_callback_t)(const void*, mgcom_index_t);

/**
 * Register a callback function as a Active Messages' handler.
 */
mgcom_error_t mgcom_register_am_handler(
    mgcom_am_handler_id_t
,   mgcom_am_handler_callback_t
);

/// Control block for sending Active Messages.
typedef struct mgcom_send_am_cb_tag {
    mgbase_async_request request;
}
mgcom_send_am_cb;

/**
 * Invoke the callback function on the specified remote node.
 */
mgcom_error_t mgcom_send_am(
    mgcom_send_am_cb*     cb
,   mgcom_am_handler_id_t id
,   const void*           value
,   mgcom_index_t         size
,   mgcom_process_id_t    dest_proc
);


/**
 * Barrier (Collective)
 */
mgcom_error_t mgcom_barrier(void) MGBASE_NOEXCEPT;


mgcom_process_id_t mgcom_current_process_id(void) MGBASE_NOEXCEPT;

mgcom_index_t mgcom_number_of_processes(void) MGBASE_NOEXCEPT;

// TODO

#define MGCOM_REGISTRATION_ALIGNMENT  4
#define MGCOM_BUFFER_ALIGNMENT        4

#ifdef __cplusplus
}
#endif

