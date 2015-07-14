
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t  mgcom_process_id_t;

typedef uint64_t  mgcom_local_addr_t;
typedef uint64_t  mgcom_remote_addr_t;

typedef uint32_t  mgcom_local_region_id_t;
typedef uint32_t  mgcom_remote_region_id_t;

typedef uint32_t  mgcom_handle_t;

typedef uint64_t  mgcom_size_t;

typedef enum {
    MGCOM_CAS_INT64,
    MGCOM_FAA_INT64
}
mgcom_operation_t;

/**
 * Initialize and start the communication.
 */
bool mgcom_initialize(int* argc, char* argv[]);

/**
 * Finalize the communication.
 */
bool mgcom_finalize(void);

/**
 * Register a region located on the current process.
 */
mgcom_local_addr_t mgcom_register_local_region(void* ptr, mgcom_size_t size, mgcom_local_region_id_t* region_id_result);

/**
 * Register a region located on a remote process.
 */
mgcom_remote_addr_t mgcom_register_remote_region(mgcom_process_id_t proc_id, void* ptr, mgcom_size_t size, mgcom_remote_region_id_t* region_id_result);

/**
 * Non-blocking contiguous "put".
 */
bool mgcom_put_async(
    mgcom_local_region_id_t local_region_id, mgcom_local_addr_t local_addr,
    mgcom_remote_region_id_t remote_region_id, mgcom_remote_addr_t remote_addr,
    mgcom_process_id_t dest_proc, mgcom_handle_t* handle
);

/**
 * Non-blocking contiguous "get".
 */
bool mgcom_get_async(
    mgcom_local_region_id_t local_region_id, mgcom_local_addr_t local_addr,
    mgcom_remote_region_id_t remote_region_id, mgcom_remote_addr_t remote_addr,
    mgcom_process_id_t src_proc, mgcom_handle_t* handle
);

typedef struct {
    mgcom_local_region_id_t  local_region_ids;
    mgcom_local_addr_t*      local_addrs;
    mgcom_remote_region_id_t remote_region_ids;
    mgcom_remote_addr_t*     remote_addrs;
    mgcom_size_t             num_addrs;
    mgcom_size_t             bytes;
}
mgcom_vector_access_t;

/**
 * Non-blocking vector "put".
 */
bool mgcom_put_vector_async(mgcom_vector_access_t* accesses, mgcom_size_t num_accesses, mgcom_process_id_t dest_proc, mgcom_handle_t* handle);

/**
 * Non-blocking vector "get".
 */
bool mgcom_get_vector_async(mgcom_vector_access_t* accesses, mgcom_size_t num_accesses, mgcom_process_id_t src_proc, mgcom_handle_t* handle);

/**
 * Non-blockng strided "put".
 */
bool mgcom_put_strided_async(
    mgcom_local_region_id_t local_region_id, mgcom_local_addr_t local_addr, mgcom_size_t* local_stride,
    mgcom_remote_region_id_t remote_region_id, mgcom_remote_addr_t remote_addr, mgcom_size_t* remote_stride,
    mgcom_size_t* count, mgcom_size_t stride_level,
    mgcom_process_id_t dest_proc, mgcom_handle_t* handle
);

/**
 * Non-blockng strided "get".
 */
bool mgcom_get_strided_async(
    mgcom_local_region_id_t local_region_id, mgcom_local_addr_t local_addr, mgcom_size_t* local_stride,
    mgcom_remote_region_id_t remote_region_id, mgcom_remote_addr_t remote_addr, mgcom_size_t* remote_stride,
    mgcom_size_t* count, mgcom_size_t stride_level,
    mgcom_process_id_t src_proc, mgcom_handle_t* handle
);

/**
 * Remote atomic operation.
 */
bool mgcom_rmw(
    mgcom_operation_t op,
    void* local_expected,
    mgcom_local_region_id_t local_region_id, mgcom_local_addr_t local_addr,
    mgcom_remote_region_id_t remote_region_id, mgcom_remote_addr_t remote_addr,
    mgcom_process_id_t dest_proc, mgcom_handle_t* handle
);

/**
 * Wait for the completion of a non-blocking operation.
 */
bool mgcom_wait(mgcom_handle_t* handle);

/**
 * Barrier (Collective)
 */
bool mgcom_barrier(void);


#ifdef __cplusplus
}
#endif

