
#pragma once

#include "mgcom.h"

namespace mgcom {

typedef ::mgcom_process_id_t        process_id_t;

typedef ::mgcom_local_addr_t        local_addr_t;
typedef ::mgcom_remote_addr_t       remote_addr_t;

typedef ::mgcom_local_region_id_t   local_region_id_t;
typedef ::mgcom_remote_region_id_t  remote_region_id_t;

typedef ::mgcom_index_t             index_t;

typedef ::mgcom_notifier_t          notifier_t;

typedef ::mgcom_local_operation_t   local_operation_t;
typedef ::mgcom_remote_operation_t  remote_operation_t;

typedef ::mgcom_vector_access_t     vector_access_t;

/**
 * Initialize and start the communication.
 */
void initialize(int* argc, char*** argv);

/**
 * Finalize the communication.
 */
void finalize();

/**
 * Register a region located on the current process.
 */
void register_local_region(
    void*              local_ptr
,   index_t            size_in_bytes
,   local_region_id_t* region_id_result
,   local_addr_t*      region_addr_result
);

/**
 * Prepare a region located on a remote process.
 */
void register_remote_region(
    process_id_t        proc_id
,   local_region_id_t   local_region_id
,   void*               remote_ptr
,   index_t             size_in_bytes
,   remote_region_id_t* region_id_result
,   remote_addr_t*      region_addr_result
);

/**
 * De-register the region located on the current process.
 */
void deregister_local_region(
    void*              local_ptr
,   index_t            size_in_size
,   local_region_id_t  local_region_id
,   local_addr_t       local_addr
);

/**
 * Non-blocking contiguous "put".
 */
bool try_put_async(
    local_region_id_t  local_region_id
,   local_addr_t       local_addr
,   remote_region_id_t remote_region_id
,   remote_addr_t      remote_addr
,   index_t            size_in_bytes
,   process_id_t       dest_proc
,   notifier_t         on_complete
);

/**
 * Non-blocking contiguous "get".
 */
bool try_get_async(
    local_region_id_t  local_region_id
,   local_addr_t       local_addr
,   remote_region_id_t remote_region_id
,   remote_addr_t      remote_addr
,   index_t            size_in_bytes
,   process_id_t       dest_proc
,   notifier_t         on_complete
);

typedef struct {
    local_region_id_t  local_region_ids;
    local_addr_t*      local_addrs;
    remote_region_id_t remote_region_ids;
    remote_addr_t*     remote_addrs;
    index_t            num_addrs;
    index_t            bytes;
}
mgcom_vector_access_t;

/**
 * Non-blocking vector "put".
 */
bool try_put_vector_async(
    vector_access_t* accesses
,   index_t          num_accesses
,   process_id_t     dest_proc
,   notifier_t       on_complete
);

/**
 * Non-blocking vector "get".
 */
bool try_get_vector_async(
    vector_access_t* accesses
,   index_t          num_accesses
,   process_id_t     src_proc
,   notifier_t       on_complete
);

/**
 * Non-blockng strided "put".
 */
bool try_put_strided_async(
    local_region_id_t  local_region_id
,   local_addr_t       local_addr
,   index_t*           local_stride
,   remote_region_id_t remote_region_id
,   remote_addr_t      remote_addr
,   index_t*           remote_stride
,   index_t*           count
,   index_t            stride_level
,   process_id_t       dest_proc
,   notifier_t         on_complete
);

/**
 * Non-blockng strided "get".
 */
bool try_get_strided_async(
    local_region_id_t  local_region_id
,   local_addr_t       local_addr
,   index_t*           local_stride
,   remote_region_id_t remote_region_id
,   remote_addr_t      remote_addr
,   index_t*           remote_stride
,   index_t*           count
,   index_t            stride_level
,   process_id_t       src_proc
,   notifier_t         on_complete
);

/**
 * Non-blocking remote atomic operation.
 */
bool try_rmw_async(
    remote_operation_t op
,   void*              local_expected
,   local_region_id_t  local_region_id
,   local_addr_t       local_addr
,   remote_region_id_t remote_region_id
,   remote_addr_t      remote_addr
,   process_id_t       dest_proc
,   notifier_t         on_complete
);

/**
 * Polling.
 */
void poll();

/**
 * Barrier (Collective)
 */
void barrier();


process_id_t current_process_id() MGBASE_NOEXCEPT;

index_t number_of_processes() MGBASE_NOEXCEPT;

}

