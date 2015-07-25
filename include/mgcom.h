
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <mgbase/lang.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t  mgcom_process_id_t;

typedef uint64_t  mgcom_local_addr_t;
typedef uint64_t  mgcom_remote_addr_t;

typedef uint32_t  mgcom_local_region_id_t;
typedef uint32_t  mgcom_remote_region_id_t;

typedef uint64_t  mgcom_index_t;

typedef enum mgcom_local_operation_tag {
    MGCOM_LOCAL_ASSIGN_INT64
,   MGCOM_LOCAL_FAA_INT64
}
mgcom_local_operation_t;

typedef struct mgcom_notifier {
    mgcom_local_operation_t op;
    void*                   pointer;
    uint64_t                value;
}
mgcom_notifier_t;

typedef enum mgcom_remote_operation_tag {
    MGCOM_REMOTE_CAS_INT64,
    MGCOM_REMOTE_FAA_INT64
}
mgcom_remote_operation_t;


enum mgcom_error_t {
    MGCOM_SUCCESS
,   MGCOM_FAILURE
};

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
mgcom_error_t mgcom_register_local_region(
    void*                    local_ptr
,   mgcom_index_t            size_in_bytes
,   mgcom_local_region_id_t* region_id_result
,   mgcom_local_addr_t*      region_addr_result
) MGBASE_NOEXCEPT;

/**
 * Prepare a region located on a remote process.
 */
mgcom_error_t mgcom_use_remote_region(
    mgcom_process_id_t        proc_id
,   mgcom_local_region_id_t   local_region_id
,   void*                     remote_ptr
,   mgcom_index_t             size_in_bytes
,   mgcom_remote_region_id_t* region_id_result
,   mgcom_remote_addr_t*      region_addr_result
) MGBASE_NOEXCEPT;

/**
 * De-register the region located on the current process.
 */
mgcom_error_t mgcom_deregister_local_region(
    void*                    local_ptr
,   mgcom_index_t            size_in_size
,   mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
) MGBASE_NOEXCEPT;

/**
 * Non-blocking contiguous "put".
 */
mgcom_error_t mgcom_try_put_async(
    mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
,   mgcom_remote_region_id_t remote_region_id
,   mgcom_remote_addr_t      remote_addr
,   mgcom_index_t            size_in_bytes
,   mgcom_process_id_t       dest_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blocking contiguous "get".
 */
mgcom_error_t mgcom_try_get_async(
    mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
,   mgcom_remote_region_id_t remote_region_id
,   mgcom_remote_addr_t      remote_addr
,   mgcom_index_t            size_in_bytes
,   mgcom_process_id_t       dest_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;

typedef struct {
    mgcom_local_region_id_t  local_region_ids;
    mgcom_local_addr_t*      local_addrs;
    mgcom_remote_region_id_t remote_region_ids;
    mgcom_remote_addr_t*     remote_addrs;
    mgcom_index_t            num_addrs;
    mgcom_index_t            bytes;
}
mgcom_vector_access_t;

/**
 * Non-blocking vector "put".
 */
mgcom_error_t mgcom_try_put_vector_async(
    mgcom_vector_access_t*   accesses
,   mgcom_index_t            num_accesses
,   mgcom_process_id_t       dest_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blocking vector "get".
 */
mgcom_error_t mgcom_try_get_vector_async(
    mgcom_vector_access_t*   accesses
,   mgcom_index_t            num_accesses
,   mgcom_process_id_t       src_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blockng strided "put".
 */
mgcom_error_t mgcom_try_put_strided_async(
    mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
,   mgcom_index_t*           local_stride
,   mgcom_remote_region_id_t remote_region_id
,   mgcom_remote_addr_t      remote_addr
,   mgcom_index_t*           remote_stride
,   mgcom_index_t*           count
,   mgcom_index_t            stride_level
,   mgcom_process_id_t       dest_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blockng strided "get".
 */
mgcom_error_t mgcom_try_get_strided_async(
    mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
,   mgcom_index_t*           local_stride
,   mgcom_remote_region_id_t remote_region_id
,   mgcom_remote_addr_t      remote_addr
,   mgcom_index_t*           remote_stride
,   mgcom_index_t*           count
,   mgcom_index_t            stride_level
,   mgcom_process_id_t       src_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blocking remote atomic operation.
 */
mgcom_error_t mgcom_try_rmw_async(
    mgcom_remote_operation_t op
,   void*                    local_expected
,   mgcom_local_region_id_t  local_region_id
,   mgcom_local_addr_t       local_addr
,   mgcom_remote_region_id_t remote_region_id
,   mgcom_remote_addr_t      remote_addr
,   mgcom_process_id_t       dest_proc
,   mgcom_notifier_t         on_complete
,   bool*                    succeeded
) MGBASE_NOEXCEPT;


/**
 * Polling.
 */
mgcom_error_t mgcom_poll(void) MGBASE_NOEXCEPT;


/**
 * Barrier (Collective)
 */
mgcom_error_t mgcom_barrier(void) MGBASE_NOEXCEPT;


mgcom_process_id_t mgcom_current_process_id(void) MGBASE_NOEXCEPT;

mgcom_index_t mgcom_number_of_processes(void) MGBASE_NOEXCEPT;

#ifdef __cplusplus
}
#endif

