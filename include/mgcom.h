
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <mgbase/lang.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t  mgcom_index_t;

typedef uint32_t  mgcom_process_id_t;

typedef uint32_t  mgcom_local_region_id_t;
typedef uint64_t  mgcom_local_region_address_t;

typedef uint32_t  mgcom_remote_region_id_t;
typedef uint64_t  mgcom_remote_region_address_t;

typedef uint64_t  mgcom_address_offset_t;

typedef struct mgcom_local_region_tag {
    mgcom_local_region_id_t       local_id;       // Used by Tofu, Infiniband
    mgcom_remote_region_id_t      remote_id;      // Used by Infiniband
    mgcom_local_region_address_t  local_address;  // Used by Tofu, Infiniband
}
mgcom_local_region_t;

typedef struct mgcom_remote_region_tag {
    mgcom_remote_region_id_t      remote_id;      // Used by Infiniband
    mgcom_remote_region_address_t remote_address; // Used by Tofu, Infiniband
}
mgcom_remote_region_t;

typedef struct mgcom_local_address_tag {
    mgcom_local_region_t          region;
    mgcom_address_offset_t        offset;
}
mgcom_local_address_t;

typedef struct mgcom_remote_region_address_tag {
    mgcom_remote_region_t         region;
    mgcom_address_offset_t        offset;
}
mgcom_remote_address_t;


typedef enum mgcom_local_operation_tag {
    MGCOM_LOCAL_ASSIGN_INT8
,   MGCOM_LOCAL_ASSIGN_INT64
,   MGCOM_LOCAL_FAA_INT64
}
mgcom_local_operation_t;

typedef struct mgcom_notifier_tag {
    mgcom_local_operation_t operation;
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
mgcom_error_t mgcom_register_region(
    void*                          local_pointer
,   mgcom_index_t                  size_in_bytes
,   mgcom_local_region_t*          result
) MGBASE_NOEXCEPT;

/**
 * Prepare a region located on a remote process.
 */
mgcom_error_t mgcom_use_remote_region(
    mgcom_process_id_t             proc_id
,   mgcom_local_region_t           local_region
,   mgcom_index_t                  size_in_bytes
,   mgcom_remote_region_t*         result
) MGBASE_NOEXCEPT;

/**
 * De-register the region located on the current process.
 */
mgcom_error_t mgcom_deregister_region(
    mgcom_local_region_t           local_region
,   void*                          local_pointer
,   mgcom_index_t                  size_in_bytes
) MGBASE_NOEXCEPT;

/**
 * Non-blocking contiguous write.
 */
mgcom_error_t mgcom_try_write_async(
    mgcom_local_address_t          local_address
,   mgcom_remote_address_t         remote_address
,   mgcom_index_t                  size_in_bytes
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blocking contiguous read.
 */
mgcom_error_t mgcom_try_read_async(
    mgcom_local_address_t          local_address
,   mgcom_remote_address_t         remote_address
,   mgcom_index_t                  size_in_bytes
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

typedef struct {
    mgcom_local_region_t           local_region;
    mgcom_address_offset_t*        local_offsets;
    mgcom_remote_region_t          remote_region;
    mgcom_address_offset_t*        remote_offsets;
    mgcom_index_t                  number_of_offsets;
    mgcom_index_t                  bytes;
}
mgcom_vector_access_t;

/**
 * Non-blocking vector write.
 */
mgcom_error_t mgcom_try_write_vector_async(
    mgcom_vector_access_t*         accesses
,   mgcom_index_t                  num_accesses
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blocking vector read.
 */
mgcom_error_t mgcom_try_read_vector_async(
    mgcom_vector_access_t*         accesses
,   mgcom_index_t                  num_accesses
,   mgcom_process_id_t             src_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blockng strided write.
 */
mgcom_error_t mgcom_try_write_strided_async(
    mgcom_local_address_t          local_address
,   mgcom_index_t*                 local_stride
,   mgcom_remote_address_t         remote_address
,   mgcom_index_t*                 remote_stride
,   mgcom_index_t*                 count
,   mgcom_index_t                  stride_level
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blockng strided read.
 */
mgcom_error_t mgcom_try_read_strided_async(
    mgcom_local_address_t          local_address
,   mgcom_index_t*                 local_stride
,   mgcom_remote_address_t         remote_address
,   mgcom_index_t*                 remote_stride
,   mgcom_index_t*                 count
,   mgcom_index_t                  stride_level
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
) MGBASE_NOEXCEPT;

/**
 * Non-blocking remote atomic operation.
 */
mgcom_error_t mgcom_try_rmw_async(
    mgcom_remote_operation_t       operation
,   void*                          local_expected
,   mgcom_local_address_t          local_address
,   mgcom_remote_address_t         remote_address
,   mgcom_process_id_t             dest_proc
,   mgcom_notifier_t               on_complete
,   bool*                          succeeded
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

