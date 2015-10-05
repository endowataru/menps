
#pragma once

#include "mgcom.h"

#include <mgbase/control.hpp>
#include <mgbase/threading/atomic.hpp>

#include <cstddef>
#include <cstring>

namespace mgcom {

typedef mgcom_index_t                  index_t;

typedef mgcom_process_id_t             process_id_t;

typedef mgcom_local_operation          local_operation;
typedef mgcom_local_notifier           local_notifier;

typedef mgcom_remote_operation         remote_operation;

/**
 * Initialize and start the communication.
 */
void initialize(int* argc, char*** argv);

/**
 * Finalize the communication.
 */
void finalize();

/// Remote Memory Access (RMA)
namespace rma {

typedef mgcom_rma_address_offset_t         address_offset_t;

typedef mgcom_rma_region_key               region_key;
typedef mgcom_rma_local_region             local_region;
typedef mgcom_rma_remote_region            remote_region;

typedef mgcom_rma_local_address            local_address;
typedef mgcom_rma_remote_address           remote_address;


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


typedef mgcom_rma_registered_buffer   registered_buffer;

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

typedef mgcom_rma_remote_read_cb    remote_read_cb;
typedef mgcom_rma_remote_write_cb   remote_write_cb;

namespace detail {

void remote_read_nb(remote_read_cb& cb);
void remote_write_nb(remote_write_cb& cb);

}

namespace {

/**
 * Non-blocking contiguous read.
 */
inline void remote_read_nb(
    remote_read_cb&       cb
,   process_id_t          proc
,   const remote_address& remote_addr
,   const local_address&  local_addr
,   index_t               size_in_bytes
) {
    mgbase::control::initialize(cb);
    cb.proc          = proc;
    cb.remote_addr   = remote_addr;
    cb.local_addr    = local_addr;
    cb.size_in_bytes = size_in_bytes;
    
    detail::remote_read_nb(cb);
}

/**
 * Non-blocking contiguous write.
 */
inline void remote_write_nb(
    remote_write_cb&      cb
,   process_id_t          proc
,   const remote_address& remote_addr
,   const local_address&  local_addr
,   index_t               size_in_bytes
) {
    mgbase::control::initialize(cb);
    cb.proc          = proc;
    cb.remote_addr   = remote_addr;
    cb.local_addr    = local_addr;
    cb.size_in_bytes = size_in_bytes;
    
    detail::remote_write_nb(cb);
}

}


typedef mgcom_rma_write_strided_cb  write_strided_cb;

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


typedef mgcom_rma_read_strided_cb  read_strided_cb;

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


// Atomic operations.

typedef mgcom_rma_atomic_default_t              atomic_default_t;

typedef mgcom_rma_atomic_read_default_cb        remote_atomic_read_default_cb;
typedef mgcom_rma_atomic_write_default_cb       remote_atomic_write_default_cb;

typedef mgcom_rma_local_compare_and_swap_default_cb     local_compare_and_swap_default_cb;
typedef mgcom_rma_local_fetch_and_add_default_cb        local_fetch_and_add_default_cb;

typedef mgcom_rma_remote_compare_and_swap_default_cb    remote_compare_and_swap_default_cb;
typedef mgcom_rma_remote_fetch_and_add_default_cb       remote_fetch_and_add_default_cb;

namespace detail {

void remote_atomic_read_default_nb(remote_atomic_read_default_cb& cb);
void remote_atomic_write_default_nb(remote_atomic_write_default_cb& cb);

void local_compare_and_swap_default_nb(local_compare_and_swap_default_cb& cb);
void local_fetch_and_add_default_nb(local_fetch_and_add_default_cb& cb);

void remote_compare_and_swap_default_nb(remote_compare_and_swap_default_cb& cb);
void remote_fetch_and_add_default_nb(remote_fetch_and_add_default_cb& cb);

}

namespace {

/**
 * Non-blocking remote atomic read.
 */
inline void remote_atomic_read_default_nb(
    remote_atomic_read_default_cb&      cb
,   process_id_t                        proc
,   const remote_address&               remote_addr
,   const local_address&                local_addr
,   const local_address&                buf_addr
) {
    mgbase::control::initialize(cb);
    cb.proc        = proc;
    cb.remote_addr = remote_addr;
    cb.local_addr  = local_addr;
    cb.buf_addr    = buf_addr;
    
    detail::remote_atomic_read_default_nb(cb);
}

/**
 * Non-blocking remote atomic write.
 */
inline void remote_atomic_write_default_nb(
    remote_atomic_write_default_cb&     cb
,   process_id_t                        proc
,   const remote_address&               remote_addr
,   const local_address&                local_addr
,   const local_address&                buf_addr
) {
    mgbase::control::initialize(cb);
    cb.proc        = proc;
    cb.remote_addr = remote_addr;
    cb.local_addr  = local_addr;
    cb.buf_addr    = buf_addr;
    
    detail::remote_atomic_write_default_nb(cb);
}


/**
 * Non-blocking local compare-and-swap.
 */
inline void local_compare_and_swap_default_nb(
    local_compare_and_swap_default_cb&  cb
,   const local_address&                target_addr
,   const local_address&                expected_addr
,   const local_address&                desired_addr
,   const local_address&                result_addr
) {
    mgbase::control::initialize(cb);
    cb.target_addr   = target_addr;
    cb.expected_addr = expected_addr;
    cb.desired_addr  = desired_addr;
    cb.result_addr   = result_addr;
    
    detail::local_compare_and_swap_default_nb(cb);
}

/**
 * Non-blocking local fetch-and-add.
 */
inline void local_fetch_and_add_default_nb(
    local_fetch_and_add_default_cb&     cb
,   const local_address&                target_addr
,   const local_address&                diff_addr
,   const local_address&                result_addr
) {
    mgbase::control::initialize(cb);
    cb.target_addr = target_addr;
    cb.diff_addr   = diff_addr;
    cb.result_addr = result_addr;
    
    detail::local_fetch_and_add_default_nb(cb);
}

/**
 * Non-blocking remote compare-and-swap.
 */
inline void remote_compare_and_swap_default_nb(
    remote_compare_and_swap_default_cb& cb
,   process_id_t                        target_proc
,   const remote_address&               target_addr
,   const local_address&                expected_addr
,   const local_address&                desired_addr
,   const local_address&                result_addr
) {
    mgbase::control::initialize(cb);
    cb.target_addr   = target_addr;
    cb.target_proc   = target_proc;
    cb.expected_addr = expected_addr;
    cb.desired_addr  = desired_addr;
    cb.result_addr   = result_addr;
    
    detail::remote_compare_and_swap_default_nb(cb);
}

/**
 * Non-blocking remote fetch-and-add.
 */
inline void remote_fetch_and_add_default_nb(
    remote_fetch_and_add_default_cb&    cb
,   process_id_t                        target_proc
,   const remote_address&               target_addr
,   const local_address&                value_addr
,   const local_address&                result_addr
) {
    mgbase::control::initialize(cb);
    cb.target_addr = target_addr;
    cb.target_proc = target_proc;
    cb.value_addr  = value_addr;
    cb.result_addr = result_addr;
    
    detail::remote_fetch_and_add_default_nb(cb);
}

}

/**
 * Polling for RDMA.
 *
 * This function is required to watch the hardware queue of the RDMA engine.
 * This function is usually called by the low-level layer,
 * but is not automatically called by mgcom.
 *
 * The queue for AM is not related to this function.
 */
void poll();

}

// Active Messages (AM)
namespace am {

typedef mgcom_am_handler_id_t        handler_id_t;

typedef mgcom_am_handler_callback_t  handler_callback_t;

typedef mgcom_am_callback_parameters callback_parameters;

void register_handler(
    handler_id_t       id
,   handler_callback_t callback
);

typedef mgcom_am_message  message;

typedef mgcom_am_send_cb  send_cb;

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
    mgbase::control::initialize(cb);
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

