
#pragma once

#include "mgcom.h"

#include <mgbase/control.hpp>
#include <mgbase/threading/atomic.hpp>

#include <cstddef>
#include <cstring>

namespace mgcom {

typedef ::mgcom_index_t                  index_t;

typedef ::mgcom_process_id_t             process_id_t;

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

namespace rma {

typedef ::mgcom_rma_address_offset_t         address_offset_t;

typedef ::mgcom_rma_region_key               region_key;
typedef ::mgcom_rma_local_region             local_region;
typedef ::mgcom_rma_remote_region            remote_region;

typedef ::mgcom_rma_local_address            local_address;
typedef ::mgcom_rma_remote_address           remote_address;


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


typedef ::mgcom_rma_registered_buffer   registered_buffer;

/**
 * Allocate a registered buffer from the buffer pool.
 */
registered_buffer allocate(index_t size_in_bytes);

/**
 * Deallocate a registered buffer allocated from the buffer pool.
 */
void deallocate(const registered_buffer& buffer);

namespace {

MGBASE_CONSTEXPR index_t registration_alignment = MGCOM_REGISTRATION_ALIGNMENT;

MGBASE_CONSTEXPR index_t buffer_alignment       = MGCOM_BUFFER_ALIGNMENT;

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

inline local_address to_address(const registered_buffer& buffer) MGBASE_NOEXCEPT {
    return buffer.addr;
}

inline local_address advanced(const local_address& addr, index_t diff) {
    local_address result = { addr.region, addr.offset + diff };
    return result;
}
inline remote_address advanced(const remote_address& addr, index_t diff) {
    remote_address result = { addr.region, addr.offset + diff };
    return result;
}

inline void* to_pointer(const local_region& region) MGBASE_NOEXCEPT {
    return region.key.pointer;
}

inline void* to_pointer(const local_address& addr) MGBASE_NOEXCEPT {
    return static_cast<mgbase::uint8_t*>(to_pointer(addr.region)) + addr.offset;
}

inline void* to_pointer(const registered_buffer& buffer) MGBASE_NOEXCEPT {
    return to_pointer(to_address(buffer));
}

inline local_region allocate_region(index_t size_in_bytes) {
    void* const ptr = new mgbase::uint8_t[size_in_bytes];
    return register_region(ptr, size_in_bytes);
}
inline void deallocate_region(const local_region& region) {
    deregister_region(region);
    delete[] static_cast<mgbase::uint8_t*>(to_pointer(region));
}

}


/**
 * Low-level function of contiguous write.
 */
bool try_write(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
);

/**
 * Low-level function of contiguous read.
 */
bool try_read(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
);


typedef ::mgcom_rma_write_cb    write_cb;

typedef ::mgcom_rma_read_cb     read_cb;

namespace detail {

void write_nb(write_cb& cb);

void read_nb(read_cb& cb);

}

namespace {

/**
 * Non-blocking contiguous write.
 */
inline void write_nb(
    write_cb&             cb
,   const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
) {
    cb.local_addr    = local_addr;
    cb.remote_addr   = remote_addr;
    cb.size_in_bytes = size_in_bytes;
    cb.dest_proc     = dest_proc;
    
    detail::write_nb(cb);
}

/**
 * Non-blocking contiguous read.
 */
inline void read_nb(
    read_cb&              cb
,   const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
) {
    cb.local_addr    = local_addr;
    cb.remote_addr   = remote_addr;
    cb.size_in_bytes = size_in_bytes;
    cb.dest_proc     = dest_proc;
    
    detail::read_nb(cb);
}

}


typedef ::mgcom_rma_write_strided_cb  write_strided_cb;

/**
 * Non-blockng strided write.
 */
void write_strided_nb(
    write_strided_cb&        cb
,   const local_address&     local_addr
,   index_t*                 local_stride
,   const remote_address&    remote_addr
,   index_t*                 remote_stride
,   index_t*                 count
,   index_t                  stride_level
,   process_id_t             dest_proc
);


typedef ::mgcom_rma_read_strided_cb  read_strided_cb;

/**
 * Non-blockng strided read.
 */
void read_strided_nb(
    read_strided_cb&        cb
,   const local_address&    local_addr
,   index_t*                local_stride
,   const remote_address&   remote_addr
,   index_t*                remote_stride
,   index_t*                count
,   index_t                 stride_level
,   process_id_t            dest_proc
);



typedef ::mgcom_rma_compare_and_swap_64_cb  compare_and_swap_64_cb;

namespace detail {

void compare_and_swap_64_nb(compare_and_swap_64_cb& cb);

}

namespace {

/**
 * Non-blocking compare-and-swap.
 */
inline void compare_and_swap_64_nb(
    compare_and_swap_64_cb& cb
,   remote_address          remote_addr
,   mgbase::uint64_t        expected
,   mgbase::uint64_t        desired
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
) {
    cb.remote_addr = remote_addr;
    cb.expected    = expected;
    cb.desired     = desired;
    cb.result      = result;
    cb.dest_proc   = dest_proc;
    
    detail::compare_and_swap_64_nb(cb);
}

}

typedef ::mgcom_rma_fetch_and_op_64_cb  fetch_and_op_64_cb;

namespace detail {

void fetch_and_add_64_nb(fetch_and_op_64_cb& cb);

}

namespace {

inline void fetch_and_add_64_nb(
    fetch_and_op_64_cb&     cb
,   const remote_address&   remote_addr
,   mgbase::uint64_t        value
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
) {
    cb.remote_addr = remote_addr;
    cb.value       = value;
    cb.result      = result;
    cb.dest_proc   = dest_proc;
    
    detail::fetch_and_add_64_nb(cb);
}

}


void poll();

}

namespace am {

typedef ::mgcom_am_handler_id_t        handler_id_t;

typedef ::mgcom_am_handler_callback_t  handler_callback_t;

typedef ::mgcom_am_callback_parameters callback_parameters;

void register_handler(
    handler_id_t       id
,   handler_callback_t callback
);

typedef ::mgcom_am_message  message;

typedef ::mgcom_am_send_cb  send_cb;

namespace detail {

void send_nb(send_cb& cb);

}

namespace {

inline void send_nb(
    send_cb&        cb
,   handler_id_t    id
,   const void*     value
,   index_t         size
,   process_id_t    dest_proc
) {
    cb.dest_proc = dest_proc;
    cb.msg.id    = id;
    cb.msg.size  = size;
    
    std::memcpy(cb.msg.data, value, size);
    
    detail::send_nb(cb);
}

}

void poll();

void reply(
    const callback_parameters* params
,   handler_id_t               id
,   const void*                value
,   index_t                    size
);

}

/**
 * Barrier (Collective)
 */
void barrier();


process_id_t current_process_id() MGBASE_NOEXCEPT;

index_t number_of_processes() MGBASE_NOEXCEPT;

namespace {


inline local_notifier make_notifier_no_operation() MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_NO_OPERATION, MGBASE_NULLPTR, 0 };
    return result;
}

inline local_notifier make_notifier_assign(bool* ptr, bool value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint8_t* ptr, mgbase::uint8_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT8, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint16_t* ptr, mgbase::uint16_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT16, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint32_t* ptr, mgbase::uint32_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT32, ptr, value };
    return result;
}
inline local_notifier make_notifier_assign(mgbase::uint64_t* ptr, mgbase::uint64_t value) MGBASE_NOEXCEPT {
    local_notifier result = { MGCOM_LOCAL_ASSIGN_INT64, ptr, value };
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

