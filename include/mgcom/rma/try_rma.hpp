
#pragma once

#include <mgcom/rma/pointer.hpp>
#include <mgbase/operation.hpp>
#include <mgbase/type_traits.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

/**
 * Try to do contiguous read.
 */
bool try_remote_read(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
);

/**
 * Try to do contiguous write.
 */
bool try_remote_write(
    process_id_t                proc
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
inline MGBASE_ALWAYS_INLINE bool try_remote_read(
    process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   const mgbase::operation&        on_complete
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT(
        (mgbase::is_runtime_sized_assignable<Local, Remote>::value)
    ,   "Breaking type safety"
    );
    
    return untyped::try_remote_read(
        proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}

/**
 * Try to do contiguous write.
 */
template <typename Remote, typename Local>
inline MGBASE_ALWAYS_INLINE bool try_remote_write(
    process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   const mgbase::operation&        on_complete
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT(
        (mgbase::is_runtime_sized_assignable<Remote, Local>::value)
    ,   "Breaking type safety"
    );
    
    return untyped::try_remote_write(
        proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}

template <typename Remote, typename Local>
inline MGBASE_ALWAYS_INLINE bool try_remote_read(
    process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   index_t                         number_of_elements
,   const mgbase::operation&        on_complete
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT(
        (mgbase::is_same<
            typename mgbase::remove_const<Remote>::type
        ,   Local
        >::value)
    ,   "Breaking type safety"
    );
    
    return untyped::try_remote_read(
        proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}

template <typename Remote, typename Local>
inline MGBASE_ALWAYS_INLINE bool try_remote_write(
    process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
,   index_t                         number_of_elements
,   const mgbase::operation&        on_complete
) {
    // Note: Important for type safety.
    MGBASE_STATIC_ASSERT(
        (mgbase::is_same<
            Remote
        ,   typename mgbase::remove_const<Local>::type
        >::value)
    ,   "Breaking type safety"
    );
    
    return untyped::try_remote_write(
        proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    );
}

} // unnamed namespace

/**
 * Try to do atomic read.
 */
bool try_remote_atomic_read(
    process_id_t                                    proc
,   const remote_pointer<const atomic_default_t>&   remote_ptr
,   const local_pointer<atomic_default_t>&          local_ptr
,   const local_pointer<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                        on_complete
);

/**
 * Try to do atomic write.
 */
bool try_remote_atomic_write(
    process_id_t                                    proc
,   const remote_pointer<atomic_default_t>&         remote_ptr
,   const local_pointer<const atomic_default_t>&    local_ptr
,   const local_pointer<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                        on_complete
);

/**
 * Try to do remote compare-and-swap.
 */
bool try_remote_compare_and_swap(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    expected_ptr
,   const local_pointer<const atomic_default_t>&    desired_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
);

/**
 * Try to do remote fetch-and-add.
 */
bool try_remote_fetch_and_add(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    value_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
);

} // namespace rma
} // namespace mgcom

