
#pragma once

#include "mgcom.h"

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
bool try_write_async(
    local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

/**
 * Non-blocking contiguous read.
 */
bool try_read_async(
    local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

/**
 * Non-blocking vector write.
 */
bool try_write_vector_async(
    vector_access_t*               accesses
,   index_t                        num_accesses
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

/**
 * Non-blocking vector read.
 */
bool try_read_vector_async(
    vector_access_t*               accesses
,   index_t                        num_accesses
,   process_id_t                   src_proc
,   notifier_t                     on_complete
);

/**
 * Non-blockng strided write.
 */
bool try_write_strided_async(
    local_address_t                local_address
,   index_t*                       local_stride
,   remote_address_t               remote_address
,   index_t*                       remote_stride
,   index_t*                       count
,   index_t                        stride_level
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

/**
 * Non-blockng strided read.
 */
bool try_read_strided_async(
    local_address_t                local_address
,   index_t*                       local_stride
,   remote_address_t               remote_address
,   index_t*                       remote_stride
,   index_t*                       count
,   index_t                        stride_level
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

/**
 * Non-blocking remote atomic operation.
 */
bool try_rmw_async(
    remote_operation_t             operation
,   void*                          local_expected
,   local_address_t                local_address
,   remote_address_t               remote_address
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
);

typedef ::mgcom_am_handler_id_t       am_handler_id_t;

typedef ::mgcom_am_handler_callback_t am_handler_callback_t;

void register_am_handler(am_handler_callback_t);

bool try_send_am_request_to(
    am_handler_id_t                id
,   const void*                    value
,   index_t                        size
,   process_id_t                   dest_proc
);

void poll_am();

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


inline notifier_t make_notifier_no_operation() {
    notifier_t result = { MGCOM_LOCAL_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline notifier_t make_notifier_assign(bool* ptr, bool value) {
    notifier_t result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}

inline notifier_t make_notifier_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
inline notifier_t make_notifier_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

inline notifier_t make_notifier_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
inline notifier_t make_notifier_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) {
    notifier_t result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

}

