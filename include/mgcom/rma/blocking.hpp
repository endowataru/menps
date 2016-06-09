
#pragma once

#include <mgcom/rma/try_rma.hpp>
#include <mgbase/ult.hpp>

namespace mgcom {
namespace rma {

// remote_read/write

/**
 * Do contiguous read asynchronously.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE
void remote_read_async(
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
    )) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Do contiguous write asynchronously.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE
void remote_write_async(
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
    )) {
        mgbase::ult::this_thread::yield();
    }
}

/**
 * Do contiguous read.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE
void remote_read(
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
    
    while (!finished.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Do contiguous write.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE
void remote_write(
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
    
    while (!finished.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}

/**
 * Try to do atomic read.
 */
MGBASE_ALWAYS_INLINE
void remote_atomic_read_async(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   atomic_default_t* const                     dest_ptr
,   const mgbase::operation&                    on_complete
) {
    while (!try_remote_atomic_read_async(
        src_proc
    ,   src_rptr
    ,   dest_ptr
    ,   on_complete
    )) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Try to do atomic write.
 */
MGBASE_ALWAYS_INLINE
void remote_atomic_write_async(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const atomic_default_t                      value
,   const mgbase::operation&                    on_complete
) {
    while (!try_remote_atomic_write_async(
        dest_proc
    ,   dest_rptr
    ,   value
    ,   on_complete
    )) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Try to do remote compare-and-swap.
 */
MGBASE_ALWAYS_INLINE void remote_compare_and_swap_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      expected_val
,   const atomic_default_t                      desired_val
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    while (!try_remote_compare_and_swap_async(
        target_proc
    ,   target_rptr
    ,   expected_val
    ,   desired_val
    ,   result_ptr
    ,   on_complete
    )) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Try to do remote fetch-and-add.
 */
MGBASE_ALWAYS_INLINE void remote_fetch_and_add_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      value
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    while (!try_remote_fetch_and_add_async(
        target_proc
    ,   target_rptr
    ,   value
    ,   result_ptr
    ,   on_complete
    )) {
        mgbase::ult::this_thread::yield();
    }
}

/**
 * Do atomic read.
 */
MGBASE_ALWAYS_INLINE
void remote_atomic_read(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   atomic_default_t* const                     dest_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_atomic_read_async(
        src_proc
    ,   src_rptr
    ,   dest_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Do atomic write.
 */
MGBASE_ALWAYS_INLINE
void remote_atomic_write(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const atomic_default_t                      value
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_atomic_write_async(
        dest_proc
    ,   dest_rptr
    ,   value
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Do remote compare-and-swap.
 */
MGBASE_ALWAYS_INLINE
void remote_compare_and_swap(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      expected_val
,   const atomic_default_t                      desired_val
,   atomic_default_t* const                     result_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_compare_and_swap_async(
        target_proc
    ,   target_rptr
    ,   expected_val
    ,   desired_val
    ,   result_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}
/**
 * Do remote fetch-and-add.
 */
MGBASE_ALWAYS_INLINE
void remote_fetch_and_add(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      value
,   atomic_default_t* const                     result_ptr
) {
    mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_fetch_and_add_async(
        target_proc
    ,   target_rptr
    ,   value
    ,   result_ptr
    ,   mgbase::make_operation_store_release(&finished, true)
    );
    
    while (!finished.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}

} // namespace rma
} // namespace mgcom

