
#pragma once

#include "mgcom.h"

#include <mgbase/event/async_request.hpp>
#include <mgbase/threading/atomic.hpp>

namespace mgcom {

typedef ::mgcom_index_t                  index_t;

typedef ::mgcom_process_id_t             process_id_t;

typedef ::mgcom_address_offset_t         address_offset_t;

typedef ::mgcom_region_key               region_key;
typedef ::mgcom_local_region             local_region;
typedef ::mgcom_remote_region            remote_region;

typedef ::mgcom_local_address            local_address;
typedef ::mgcom_remote_address           remote_address;

typedef ::mgcom_local_operation          local_operation;
typedef ::mgcom_local_notifier           local_notifier;

typedef ::mgcom_remote_operation         remote_operation;

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
local_region register_region(
    void*   local_pointer
,   index_t size_in_bytes
);

/**
 * Prepare a region located on a remote process.
 */
remote_region use_remote_region(
    process_id_t proc_id
,   region_key   key
,   index_t      size_in_bytes
);

/**
 * De-register the region located on the current process.
 */
void deregister_region(
    local_region   region
,   void*          local_pointer
,   index_t        size_in_bytes
);



/**
 * Low-level function of contiguous write.
 */
bool try_write_async(
    local_address  local_addr
,   remote_address remote_addr
,   index_t        size_in_bytes
,   process_id_t   dest_proc
,   local_notifier on_complete
);

/**
 * Low-level function of contiguous read.
 */
bool try_read_async(
    local_address  local_addr
,   remote_address remote_addr
,   index_t        size_in_bytes
,   process_id_t   dest_proc
,   local_notifier on_complete
);



typedef ::mgcom_write_async_buffer  write_async_buffer;

/**
 * Non-blocking contiguous write.
 */
void write_async(
    write_async_buffer* buffer
,   local_address       local_addr
,   remote_address      remote_addr
,   index_t             size_in_bytes
,   process_id_t        dest_proc
);

typedef ::mgcom_read_async_buffer  read_async_buffer;

/**
 * Non-blocking contiguous read.
 */
void read_async(
    read_async_buffer* buffer
,   local_address      local_addr
,   remote_address     remote_addr
,   index_t            size_in_bytes
,   process_id_t       dest_proc
);


typedef ::mgcom_write_strided_async_buffer  write_strided_async_buffer;

/**
 * Non-blockng strided write.
 */
void write_strided_async(
    write_strided_async_buffer* buffer
,   local_address               local_addr
,   index_t*                    local_stride
,   remote_address              remote_addr
,   index_t*                    remote_stride
,   index_t*                    count
,   index_t                     stride_level
,   process_id_t                dest_proc
);


typedef ::mgcom_read_strided_async_buffer  read_strided_async_buffer;

/**
 * Non-blockng strided read.
 */
void read_strided_async(
    read_strided_async_buffer* buffer
,   local_address              local_addr
,   index_t*                   local_stride
,   remote_address             remote_addr
,   index_t*                   remote_stride
,   index_t*                   count
,   index_t                    stride_level
,   process_id_t               dest_proc
);


typedef ::mgcom_rmw_async_buffer  rmw_async_buffer;

/**
 * Non-blocking remote atomic operation.
 */
void rmw_async(
    rmw_async_buffer* buffer
,   remote_operation  operation
,   void*             local_expected
,   local_address     local_addr
,   remote_address    remote_addr
,   process_id_t      dest_proc
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


void poll();


/**
 * Barrier (Collective)
 */
void barrier();


process_id_t current_process_id() MGBASE_NOEXCEPT;

index_t number_of_processes() MGBASE_NOEXCEPT;

namespace {

inline local_notifier make_notifier_no_operation() {
    local_notifier result = { MGCOM_LOCAL_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline local_notifier make_notifier_assign(bool* ptr, bool value) {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}

inline local_notifier make_notifier_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
inline local_notifier make_notifier_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

inline local_notifier make_notifier_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
inline local_notifier make_notifier_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

}

}

