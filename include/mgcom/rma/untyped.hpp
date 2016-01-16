
#pragma once

#include <mgcom/common.hpp>
#include <mgcom/rma/untyped.h>

namespace mgcom {
namespace rma {

// Contiguous RMA operations.

typedef mgcom_rma_remote_read_cb    remote_read_cb;
typedef mgcom_rma_remote_write_cb   remote_write_cb;

// Atomic operations.

typedef mgcom_rma_atomic_default_t              atomic_default_t;

typedef mgcom_rma_atomic_read_default_cb        remote_atomic_read_default_cb;
typedef mgcom_rma_atomic_write_default_cb       remote_atomic_write_default_cb;

typedef mgcom_rma_local_compare_and_swap_default_cb     local_compare_and_swap_default_cb;
typedef mgcom_rma_local_fetch_and_add_default_cb        local_fetch_and_add_default_cb;

typedef mgcom_rma_remote_compare_and_swap_default_cb    remote_compare_and_swap_default_cb;
typedef mgcom_rma_remote_fetch_and_add_default_cb       remote_fetch_and_add_default_cb;

namespace /*unnamed*/ {

MGBASE_CONSTEXPR index_t registration_alignment = MGCOM_REGISTRATION_ALIGNMENT;

MGBASE_CONSTEXPR index_t buffer_alignment       = MGCOM_BUFFER_ALIGNMENT;

} // unnamed namespace

namespace untyped {

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

namespace /*unnamed*/ {

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

inline local_address advanced(const local_address& addr, index_t diff) MGBASE_NOEXCEPT {
    local_address result = { addr.region, addr.offset + diff };
    return result;
}
inline remote_address advanced(const remote_address& addr, index_t diff) MGBASE_NOEXCEPT {
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

inline remote_address use_remote_address(process_id_t proc_id, const local_address& addr) {
    remote_address result = { use_remote_region(proc_id, addr.region.key), addr.offset };
    return result;
}

} // unnamed namespace

namespace detail {

mgbase::deferred<void> remote_read(remote_read_cb& cb);
mgbase::deferred<void> remote_write(remote_write_cb& cb);

} // namespace detail

namespace /*unnamed*/ {

/**
 * Non-blocking contiguous read.
 */
inline mgbase::deferred<void> remote_read_nb(
    remote_read_cb&       cb
,   process_id_t          proc
,   const remote_address& remote_addr
,   const local_address&  local_addr
,   index_t               size_in_bytes
) {
    cb.proc          = proc;
    cb.remote_addr   = remote_addr;
    cb.local_addr    = local_addr;
    cb.size_in_bytes = size_in_bytes;
    
    return detail::remote_read(cb);
}

/**
 * Non-blocking contiguous write.
 */
inline mgbase::deferred<void> remote_write_nb(
    remote_write_cb&      cb
,   process_id_t          proc
,   const remote_address& remote_addr
,   const local_address&  local_addr
,   index_t               size_in_bytes
) {
    cb.proc          = proc;
    cb.remote_addr   = remote_addr;
    cb.local_addr    = local_addr;
    cb.size_in_bytes = size_in_bytes;
    
    return detail::remote_write(cb);
}

} // unnamed namespace


typedef mgcom_rma_write_strided_cb  write_strided_cb;

/**
 * Non-blockng strided write.
 */
mgbase::deferred<void> write_strided_nb(
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
mgbase::deferred<void> read_strided_nb(
    read_strided_cb&        cb
,   const local_address&    local_addr
,   index_t*                local_stride
,   const remote_address&   remote_addr
,   index_t*                remote_stride
,   index_t*                count
,   index_t                 stride_level
,   process_id_t            dest_proc
);


// Atomic operations

namespace detail {

mgbase::deferred<void> remote_atomic_read_default(remote_atomic_read_default_cb& cb);
mgbase::deferred<void> remote_atomic_write_default(remote_atomic_write_default_cb& cb);

mgbase::deferred<void> local_compare_and_swap_default(local_compare_and_swap_default_cb& cb);
mgbase::deferred<void> local_fetch_and_add_default(local_fetch_and_add_default_cb& cb);

mgbase::deferred<void> remote_compare_and_swap_default(remote_compare_and_swap_default_cb& cb);
mgbase::deferred<void> remote_fetch_and_add_default(remote_fetch_and_add_default_cb& cb);

} // namespace detail

namespace /*unnamed*/ {

/**
 * Non-blocking remote atomic read.
 */
inline mgbase::deferred<void> remote_atomic_read_default_nb(
    remote_atomic_read_default_cb&      cb
,   process_id_t                        proc
,   const remote_address&               remote_addr
,   const local_address&                local_addr
,   const local_address&                buf_addr
) {
    cb.proc        = proc;
    cb.remote_addr = remote_addr;
    cb.local_addr  = local_addr;
    cb.buf_addr    = buf_addr;
    
    return detail::remote_atomic_read_default(cb);
}

/**
 * Non-blocking remote atomic write.
 */
inline mgbase::deferred<void> remote_atomic_write_default_nb(
    remote_atomic_write_default_cb&     cb
,   process_id_t                        proc
,   const remote_address&               remote_addr
,   const local_address&                local_addr
,   const local_address&                buf_addr
) {
    cb.proc        = proc;
    cb.remote_addr = remote_addr;
    cb.local_addr  = local_addr;
    cb.buf_addr    = buf_addr;
    
    return detail::remote_atomic_write_default(cb);
}


/**
 * Non-blocking local compare-and-swap.
 */
inline mgbase::deferred<void> local_compare_and_swap_default_nb(
    local_compare_and_swap_default_cb&  cb
,   const local_address&                target_addr
,   const local_address&                expected_addr
,   const local_address&                desired_addr
,   const local_address&                result_addr
) {
    cb.target_addr   = target_addr;
    cb.expected_addr = expected_addr;
    cb.desired_addr  = desired_addr;
    cb.result_addr   = result_addr;
    
    return detail::local_compare_and_swap_default(cb);
}

/**
 * Non-blocking local fetch-and-add.
 */
inline mgbase::deferred<void> local_fetch_and_add_default_nb(
    local_fetch_and_add_default_cb&     cb
,   const local_address&                target_addr
,   const local_address&                value_addr
,   const local_address&                result_addr
) {
    cb.target_addr = target_addr;
    cb.value_addr  = value_addr;
    cb.result_addr = result_addr;
    
    return detail::local_fetch_and_add_default(cb);
}

/**
 * Non-blocking remote compare-and-swap.
 */
inline mgbase::deferred<void> remote_compare_and_swap_default_nb(
    remote_compare_and_swap_default_cb& cb
,   process_id_t                        target_proc
,   const remote_address&               target_addr
,   const local_address&                expected_addr
,   const local_address&                desired_addr
,   const local_address&                result_addr
) {
    cb.target_addr   = target_addr;
    cb.target_proc   = target_proc;
    cb.expected_addr = expected_addr;
    cb.desired_addr  = desired_addr;
    cb.result_addr   = result_addr;
    
    return detail::remote_compare_and_swap_default(cb);
}

/**
 * Non-blocking remote fetch-and-add.
 */
inline mgbase::deferred<void> remote_fetch_and_add_default_nb(
    remote_fetch_and_add_default_cb&    cb
,   process_id_t                        target_proc
,   const remote_address&               target_addr
,   const local_address&                value_addr
,   const local_address&                result_addr
) {
    cb.target_addr = target_addr;
    cb.target_proc = target_proc;
    cb.value_addr  = value_addr;
    cb.result_addr = result_addr;
    
    return detail::remote_fetch_and_add_default(cb);
}

} // unnamed namespace

} // namespace untyped

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

namespace /*unnamed*/ {

inline mgbase::uint64_t to_integer(const untyped::remote_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(static_cast<mgbase::uint8_t*>(addr.region.key.pointer) + addr.offset);
}
inline mgbase::uint64_t to_integer(const untyped::local_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(static_cast<mgbase::uint8_t*>(untyped::to_pointer(addr.region)) + addr.offset);
}

} // unnamed namespace

} // namespace rma
} // namespace mgcom

