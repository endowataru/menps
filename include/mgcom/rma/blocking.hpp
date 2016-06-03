
#pragma once

#include <mgcom/rma/try_rma.hpp>

namespace mgcom {
namespace rma {

// remote_read/write

/**
 * Do contiguous read asynchronously.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE void remote_read_async(
    const process_id_t          src_proc
,   const remote_ptr<Remote>&   src_rptr
,   const local_ptr<Local>&     dest_lptr
,   const index_t               number_of_elements
,   const mgbase::operation&    on_complete
) {
    while (!try_remote_read_async(
        src_proc
    ,   src_rptr
    ,   dest_lptr
    ,   number_of_elements
    ,   on_complete
    ))
    { }
}
/**
 * Do contiguous write asynchronously.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE void remote_write_async(
    const process_id_t          dest_proc
,   const remote_ptr<Remote>&   dest_rptr
,   const local_ptr<Local>&     src_lptr
,   const index_t               number_of_elements
,   const mgbase::operation&    on_complete
) {
    while (!try_remote_write_async(
        dest_proc
    ,   dest_rptr
    ,   src_lptr
    ,   number_of_elements
    ,   on_complete
    ))
    { }
}

/**
 * Do contiguous read.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE void remote_read(
    const process_id_t          src_proc
,   const remote_ptr<Remote>&   src_rptr
,   const local_ptr<Local>&     dest_lptr
,   const index_t               number_of_elements
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_read_async(
        src_proc
    ,   src_rptr
    ,   dest_lptr
    ,   number_of_elements
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) { }
}
/**
 * Do contiguous write.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE void remote_write(
    const process_id_t          dest_proc
,   const remote_ptr<Remote>&   dest_rptr
,   const local_ptr<Local>&     src_lptr
,   const index_t               number_of_elements
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_write_async(
        dest_proc
    ,   dest_rptr
    ,   src_lptr
    ,   number_of_elements
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) { }
}

/**
 * Try to do atomic read.
 */
MGBASE_ALWAYS_INLINE void remote_atomic_read_async(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   const local_ptr<atomic_default_t>&          dest_lptr
,   const local_ptr<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                    on_complete
) {
    while (!try_remote_atomic_read_async(
        src_proc
    ,   src_rptr
    ,   dest_lptr
    ,   buf_ptr
    ,   on_complete
    ))
    { }
}
/**
 * Try to do atomic write.
 */
MGBASE_ALWAYS_INLINE void remote_atomic_write_async(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const local_ptr<const atomic_default_t>&    src_lptr
,   const local_ptr<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                    on_complete
) {
    while (!try_remote_atomic_write_async(
        dest_proc
    ,   dest_rptr
    ,   src_lptr
    ,   buf_ptr
    ,   on_complete
    ))
    { }
}
/**
 * Try to do remote compare-and-swap.
 */
MGBASE_ALWAYS_INLINE void remote_compare_and_swap_async(
    process_id_t                                    target_proc
,   const remote_ptr<atomic_default_t>&         target_ptr
,   const local_ptr<const atomic_default_t>&    expected_ptr
,   const local_ptr<const atomic_default_t>&    desired_ptr
,   const local_ptr<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    while (!try_remote_compare_and_swap_async(
        target_proc
    ,   target_ptr
    ,   expected_ptr
    ,   desired_ptr
    ,   result_ptr
    ,   on_complete
    ))
    { }
}
/**
 * Try to do remote fetch-and-add.
 */
MGBASE_ALWAYS_INLINE void remote_fetch_and_add_async(
    process_id_t                                    target_proc
,   const remote_ptr<atomic_default_t>&         target_ptr
,   const local_ptr<const atomic_default_t>&    value_ptr
,   const local_ptr<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    while (!try_remote_fetch_and_add_async(
        target_proc
    ,   target_ptr
    ,   value_ptr
    ,   result_ptr
    ,   on_complete
    ))
    { }
}

/**
 * Do atomic read.
 */
MGBASE_ALWAYS_INLINE void remote_atomic_read(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   const local_ptr<atomic_default_t>&          dest_lptr
,   const local_ptr<atomic_default_t>&          buf_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_atomic_read_async(
        src_proc
    ,   src_rptr
    ,   dest_lptr
    ,   buf_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) { }
}
/**
 * Do atomic write.
 */
MGBASE_ALWAYS_INLINE void remote_atomic_write(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const local_ptr<const atomic_default_t>&    src_lptr
,   const local_ptr<atomic_default_t>&          buf_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_atomic_write_async(
        dest_proc
    ,   dest_rptr
    ,   src_lptr
    ,   buf_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) { }
}
/**
 * Do remote compare-and-swap.
 */
MGBASE_ALWAYS_INLINE void remote_compare_and_swap(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_ptr
,   const local_ptr<const atomic_default_t>&    expected_ptr
,   const local_ptr<const atomic_default_t>&    desired_ptr
,   const local_ptr<atomic_default_t>&          result_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_compare_and_swap_async(
        target_proc
    ,   target_ptr
    ,   expected_ptr
    ,   desired_ptr
    ,   result_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) { }
}
/**
 * Do remote fetch-and-add.
 */
MGBASE_ALWAYS_INLINE void remote_fetch_and_add(
    process_id_t                                    target_proc
,   const remote_ptr<atomic_default_t>&         target_ptr
,   const local_ptr<const atomic_default_t>&    value_ptr
,   const local_ptr<atomic_default_t>&          result_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_fetch_and_add_async(
        target_proc
    ,   target_ptr
    ,   value_ptr
    ,   result_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) { }
}

} // namespace rma
} // namespace mgcom

