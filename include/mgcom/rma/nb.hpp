
#pragma once

#include <mgcom/rma/pointer.hpp>
#include <mgcom/rma/nb.h>
#include <mgbase/deferred.hpp>

namespace mgcom {
namespace rma {

// DEPRECATED: These functions will be replaced by try_*.

// Contiguous RMA operations.

typedef mgcom_rma_remote_read_cb    remote_read_cb;
typedef mgcom_rma_remote_write_cb   remote_write_cb;

// Atomic operations.

typedef mgcom_rma_atomic_read_default_cb        remote_atomic_read_default_cb;
typedef mgcom_rma_atomic_write_default_cb       remote_atomic_write_default_cb;

typedef mgcom_rma_local_compare_and_swap_default_cb     local_compare_and_swap_default_cb;
typedef mgcom_rma_local_fetch_and_add_default_cb        local_fetch_and_add_default_cb;

typedef mgcom_rma_remote_compare_and_swap_default_cb    remote_compare_and_swap_default_cb;
typedef mgcom_rma_remote_fetch_and_add_default_cb       remote_fetch_and_add_default_cb;

namespace untyped {

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

#if 0

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

#endif

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


#if 0
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
#endif

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

namespace /*unnamed*/ {

/// Simple remote read.
template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    mgbase::is_runtime_sized_assignable<Local, Remote>::value
,   mgbase::deferred<void>
>::type
remote_read_nb(
    rma::remote_read_cb&            cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
) {
    return mgcom::rma::untyped::remote_read_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   mgbase::runtime_size_of<Remote>()
    );
}

/// Simple remote write.
template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    mgbase::is_runtime_sized_assignable<Remote, Local>::value
,   mgbase::deferred<void>
>::type
remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
) {
    return mgcom::rma::untyped::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   mgbase::runtime_size_of<Local>()
    );
}

template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    mgbase::is_same<
        typename mgbase::remove_const<Remote>::type
    ,   Local
    >::value
,   mgbase::deferred<void>
>::type
remote_read_nb(
    remote_read_cb&                 cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   index_t                         number_of_elements
) {
    return mgcom::rma::untyped::remote_read_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    );
}

template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    mgbase::is_same<
        Remote
    ,   typename mgbase::remove_const<Local>::type
    >::value
,   mgbase::deferred<void>
>::type
remote_write_nb(
    remote_write_cb&                cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   index_t                         number_of_elements
) {
    return mgcom::rma::untyped::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    );
}

inline mgbase::deferred<void> remote_atomic_read_default_nb(
    remote_atomic_read_default_cb&                      cb
,   process_id_t                                        proc
,   const remote_pointer<const rma::atomic_default_t>&  remote_ptr
,   const local_pointer<rma::atomic_default_t>&         local_ptr
,   const local_pointer<rma::atomic_default_t>&         buf_ptr
) {
    return mgcom::rma::untyped::remote_atomic_read_default_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   buf_ptr.to_address()
    );
}

inline mgbase::deferred<void> remote_atomic_write_default_nb(
    remote_atomic_write_default_cb&                     cb
,   process_id_t                                        proc
,   const remote_pointer<rma::atomic_default_t>&        remote_ptr
,   const local_pointer<const rma::atomic_default_t>&   local_ptr
,   const local_pointer<rma::atomic_default_t>&         buf_ptr
) {
    return mgcom::rma::untyped::remote_atomic_write_default_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   buf_ptr.to_address()
    );
}

#if 0
/**
 * Non-blocking local compare-and-swap.
 */
inline mgbase::deferred<void> local_compare_and_swap_default_nb(
    local_compare_and_swap_default_cb&                  cb
,   const local_pointer<rma::atomic_default_t>&         target_ptr
,   const local_pointer<const rma::atomic_default_t>&   expected_ptr
,   const local_pointer<const rma::atomic_default_t>&   desired_ptr
,   const local_pointer<rma::atomic_default_t>&         result_ptr
) {
    return mgcom::rma::untyped::local_compare_and_swap_default_nb(
        cb
    ,   target_ptr.to_address()
    ,   expected_ptr.to_address()
    ,   desired_ptr.to_address()
    ,   result_ptr.to_address()
    );
}
#endif

/**
 * Non-blocking remote compare-and-swap.
 */
inline mgbase::deferred<void> remote_compare_and_swap_default_nb(
    remote_compare_and_swap_default_cb&                 cb
,   process_id_t                                        target_proc
,   const remote_pointer<rma::atomic_default_t>&        target_ptr
,   const local_pointer<const rma::atomic_default_t>&   expected_ptr
,   const local_pointer<const rma::atomic_default_t>&   desired_ptr
,   const local_pointer<rma::atomic_default_t>&         result_ptr
) {
    return mgcom::rma::untyped::remote_compare_and_swap_default_nb(
        cb
    ,   target_proc
    ,   target_ptr.to_address()
    ,   expected_ptr.to_address()
    ,   desired_ptr.to_address()
    ,   result_ptr.to_address()
    );
}

/**
 * Non-blocking remote fetch-and-add.
 */
inline mgbase::deferred<void> remote_fetch_and_add_default_nb(
    remote_fetch_and_add_default_cb&                    cb
,   process_id_t                                        target_proc
,   const remote_pointer<rma::atomic_default_t>&        target_ptr
,   const local_pointer<const rma::atomic_default_t>&   value_ptr
,   const local_pointer<rma::atomic_default_t>&         result_ptr
) {
    return mgcom::rma::untyped::remote_fetch_and_add_default_nb(
        cb
    ,   target_proc
    ,   target_ptr.to_address()
    ,   value_ptr.to_address()
    ,   result_ptr.to_address()
    );
}

} // unnamed namespace

} // namespace rma
} // namespace mgcom

