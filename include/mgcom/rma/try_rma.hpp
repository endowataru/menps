
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
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
);
/**
 * Try to do contiguous write.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_write_async(
    process_id_t                dest_proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
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
    const process_id_t              proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   const index_t                   number_of_elements
,   const mgbase::operation&        on_complete
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
        proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
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
    const process_id_t              proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   const index_t                   number_of_elements
,   const mgbase::operation&        on_complete
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
        proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}

} // unnamed namespace

/**
 * Try to start atomic read.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_read_async(
    process_id_t                                    target_proc
,   const remote_pointer<const atomic_default_t>&   remote_ptr
,   const local_pointer<atomic_default_t>&          local_ptr
,   const local_pointer<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                        on_complete
);

/**
 * Try to start atomic write.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_write_async(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         remote_ptr
,   const local_pointer<const atomic_default_t>&    local_ptr
,   const local_pointer<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                        on_complete
);

/**
 * Try to start remote compare-and-swap.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_compare_and_swap_async(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    expected_ptr
,   const local_pointer<const atomic_default_t>&    desired_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
);

/**
 * Try to start remote fetch-and-add.
 */
MGBASE_WARN_UNUSED_RESULT bool try_remote_fetch_and_add_async(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    value_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
);

} // namespace rma
} // namespace mgcom

