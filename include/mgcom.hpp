
#pragma once

#include "mgcom.h"

#include <mgbase/event/async_request.hpp>
#include <mgbase/threading/atomic.hpp>

namespace mgcom {

typedef ::mgcom_index_t                  index_t;

typedef ::mgcom_process_id_t             process_id_t;

typedef ::mgcom_address_offset_t         address_offset_t;

typedef ::mgcom_region_key_t             region_key_t;
typedef ::mgcom_local_region_t           local_region_t;
typedef ::mgcom_remote_region_t          remote_region_t;

typedef ::mgcom_local_address_t          local_address_t;
typedef ::mgcom_remote_address_t         remote_address_t;

typedef ::mgcom_local_operation_t        local_operation_t;
typedef ::mgcom_notifier_t               notifier_t;

typedef ::mgcom_remote_operation_t       remote_operation_t;

typedef ::mgcom_vector_access_t          vector_access_t;

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
local_region_t register_region(
    void*                          local_pointer
,   index_t                        size_in_bytes
);

/**
 * Prepare a region located on a remote process.
 */
remote_region_t use_remote_region(
    process_id_t                   proc_id
,   region_key_t                   key
,   index_t                        size_in_bytes
);

/**
 * De-register the region located on the current process.
 */
void deregister_region(
    local_region_t                 local_region
,   void*                          local_pointer
,   index_t                        size_in_bytes
);

/**
 * Non-blocking contiguous write.
 */
void write_async(
    mgbase::async_request*         request
,   local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
);

/**
 * Non-blocking contiguous read.
 */
void read_async(
    mgbase::async_request*         request
,   local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
);

/**
 * Non-blocking vector write.
 */
void write_vector_async(
    mgbase::async_request*         request
,   vector_access_t*               accesses
,   index_t                        num_accesses
,   process_id_t                   dest_proc
);

/**
 * Non-blocking vector read.
 */
void read_vector_async(
    mgbase::async_request*         request
,   vector_access_t*               accesses
,   index_t                        num_accesses
,   process_id_t                   src_proc
);

/**
 * Non-blockng strided write.
 */
void write_strided_async(
    mgbase::async_request*         request
,   local_address_t                local_address
,   index_t*                       local_stride
,   remote_address_t               remote_address
,   index_t*                       remote_stride
,   index_t*                       count
,   index_t                        stride_level
,   process_id_t                   dest_proc
);

/**
 * Non-blockng strided read.
 */
void read_strided_async(
    mgbase::async_request*         request
,   local_address_t                local_address
,   index_t*                       local_stride
,   remote_address_t               remote_address
,   index_t*                       remote_stride
,   index_t*                       count
,   index_t                        stride_level
,   process_id_t                   dest_proc
);

/**
 * Non-blocking remote atomic operation.
 */
void try_rmw_async(
    mgbase::async_request*         request
,   remote_operation_t             operation
,   void*                          local_expected
,   local_address_t                local_address
,   remote_address_t               remote_address
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

typedef ::mgcom_am_handler_id_t       am_handler_id_t;

typedef ::mgcom_am_handler_callback_t am_handler_callback_t;

void register_am_handler(am_handler_callback_t);

void send_am_request_to(
    mgbase::async_request*         request
,   am_handler_id_t                id
,   const void*                    value
,   index_t                        size
,   process_id_t                   dest_proc
);

/**
 * Barrier (Collective)
 */
void barrier();


process_id_t current_process_id() MGBASE_NOEXCEPT;

index_t number_of_processes() MGBASE_NOEXCEPT;



}

