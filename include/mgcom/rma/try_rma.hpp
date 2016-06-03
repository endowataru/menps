
#pragma once

#include <mgcom/rma/pointer.hpp>
#include <mgbase/operation.hpp>
#include <mgbase/type_traits.hpp>
#include <mgbase/optional.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

/**
 * Try to do contiguous read.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_read_async(
    process_id_t                src_proc
,   const remote_address&       src_raddr
,   const local_address&        dest_laddr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
);
/**
 * Try to do contiguous write.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_write_async(
    process_id_t                dest_proc
,   const remote_address&       dest_raddr
,   const local_address&        src_laddr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
);

} // namespace untyped

namespace /*unnamed*/ {

/**
 * Try to do contiguous read.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_remote_read_async(
    const process_id_t          src_proc
,   const remote_ptr<Remote>&   src_rptr
,   const local_ptr<Local>&     dest_lptr
,   const index_t               number_of_elements
,   const mgbase::operation&    on_complete
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT_MSG(
        (mgbase::is_same<
            typename mgbase::remove_const<Remote>::type
        ,   Local
        >::value)
    ,   "Breaking type safety"
    );
    
    return untyped::try_remote_read_async(
        src_proc
    ,   src_rptr.to_address()
    ,   dest_lptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}
/**
 * Try to do contiguous write.
 */
template <typename Remote, typename Local>
MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_remote_write_async(
    const process_id_t          dest_proc
,   const remote_ptr<Remote>&   dest_rptr
,   const local_ptr<Local>&     src_lptr
,   const index_t               number_of_elements
,   const mgbase::operation&    on_complete
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT_MSG(
        (mgbase::is_same<
            Remote
        ,   typename mgbase::remove_const<Local>::type
        >::value)
    ,   "Breaking type safety"
    );
    
    return untyped::try_remote_write_async(
        dest_proc
    ,   dest_rptr.to_address()
    ,   src_lptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}

} // unnamed namespace

/**
 * Try to start atomic read.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_read_async(
    process_id_t                                src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   const local_ptr<atomic_default_t>&          dest_lptr
,   const local_ptr<atomic_default_t>&          buf_lptr
,   const mgbase::operation&                    on_complete
);

/**
 * Try to start atomic write.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_write_async(
    process_id_t                                dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const local_ptr<const atomic_default_t>&    src_lptr
,   const local_ptr<atomic_default_t>&          buf_lptr
,   const mgbase::operation&                    on_complete
);

/**
 * Try to start remote compare-and-swap.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_compare_and_swap_async(
    process_id_t                                target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const local_ptr<const atomic_default_t>&    expected_lptr
,   const local_ptr<const atomic_default_t>&    desired_lptr
,   const local_ptr<atomic_default_t>&          result_lptr
,   const mgbase::operation&                    on_complete
);

/**
 * Try to start remote fetch-and-add.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_fetch_and_add_async(
    process_id_t                                target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const local_ptr<const atomic_default_t>&    value_lptr
,   const local_ptr<atomic_default_t>&          result_lptr
,   const mgbase::operation&                    on_complete
);

} // namespace rma
} // namespace mgcom

