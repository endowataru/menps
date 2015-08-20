
#pragma once

#include "mgcom.h"

#include <mgbase/event/async_request.hpp>
#include <mgbase/threading/atomic.hpp>

#include <cstddef>
#include <cstring>

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
    process_id_t      proc_id
,   const region_key& key
);

/**
 * De-register the region located on the current process.
 */
void deregister_region(const local_region& region);

namespace {

inline region_key to_region_key(const local_region& region) MGBASE_NOEXCEPT {
    return region.key;
}

inline local_address to_address(const local_region& region) MGBASE_NOEXCEPT {
    local_address addr = { region, 0 };
    return addr;
}

inline remote_address to_address(const remote_region& region) MGBASE_NOEXCEPT {
    remote_address addr = { region, 0 };
    return addr;
}

inline remote_address advance(const remote_address& addr, index_t diff) {
    remote_address result = { addr.region, addr.offset + diff };
    return result;
}

inline void* to_pointer(const local_region& region) MGBASE_NOEXCEPT {
    return region.key.pointer;
}

inline void* to_pointer(const local_address& addr) MGBASE_NOEXCEPT {
    return static_cast<mgbase::uint8_t*>(to_pointer(addr.region)) + addr.offset;
}

}


/**
 * Low-level function of contiguous write.
 */
bool try_write_async(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
);

/**
 * Low-level function of contiguous read.
 */
bool try_read_async(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
);



typedef ::mgcom_write_cb  write_cb;

/**
 * Non-blocking contiguous write.
 */
void write_async(
    write_cb*             cb
,   const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
);

typedef ::mgcom_read_cb  read_cb;

/**
 * Non-blocking contiguous read.
 */
void read_async(
    read_cb*              cb
,   const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
);


typedef ::mgcom_write_strided_cb  write_strided_cb;

/**
 * Non-blockng strided write.
 */
void write_strided_async(
    write_strided_cb* cb
,   const local_address&     local_addr
,   index_t*                 local_stride
,   const remote_address&    remote_addr
,   index_t*                 remote_stride
,   index_t*                 count
,   index_t                  stride_level
,   process_id_t             dest_proc
);


typedef ::mgcom_read_strided_cb  read_strided_cb;

/**
 * Non-blockng strided read.
 */
void read_strided_async(
    read_strided_cb* cb
,   const local_address&    local_addr
,   index_t*                local_stride
,   const remote_address&   remote_addr
,   index_t*                remote_stride
,   index_t*                count
,   index_t                 stride_level
,   process_id_t            dest_proc
);


typedef ::mgcom_rmw_cb  rmw_cb;

/**
 * Non-blocking remote atomic operation.
 */
void rmw_async(
    rmw_cb*                 cb
,   remote_operation        operation
,   void*                   local_expected
,   const local_address&    local_addr
,   const remote_address&   remote_addr
,   process_id_t            dest_proc
);

typedef ::mgcom_am_handler_id_t       am_handler_id_t;

typedef ::mgcom_am_handler_callback_t am_handler_callback_t;

void register_am_handler(am_handler_callback_t);

typedef ::mgcom_send_am_cb  send_am_cb;

void send_am(
    send_am_cb*     cb
,   am_handler_id_t id
,   const void*     value
,   index_t         size
,   process_id_t    dest_proc
);


void poll();


/**
 * Barrier (Collective)
 */
void barrier();


process_id_t current_process_id() MGBASE_NOEXCEPT;

index_t number_of_processes() MGBASE_NOEXCEPT;

namespace {

MGBASE_CONSTEXPR index_t registration_alignment = MGCOM_REGISTRATION_ALIGNMENT;

MGBASE_CONSTEXPR index_t buffer_alignment       = MGCOM_BUFFER_ALIGNMENT;


inline local_notifier make_notifier_no_operation() MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline local_notifier make_notifier_assign(bool* ptr, bool value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}

inline local_notifier make_notifier_fetch_add(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, diff };
    return result;
}
inline local_notifier make_notifier_fetch_sub(mgbase::atomic<mgbase::uint32_t>* ptr, mgbase::uint32_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT32, ptr, -diff };
    return result;
}

inline local_notifier make_notifier_fetch_add(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, diff };
    return result;
}
inline local_notifier make_notifier_fetch_sub(mgbase::atomic<mgbase::uint64_t>* ptr, mgbase::uint64_t diff) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ATOMIC_FETCH_ADD_INT64, ptr, -diff };
    return result;
}

}

}

