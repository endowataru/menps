
#pragma once

#include "requester.hpp"

namespace mgcom {
namespace rma {

namespace untyped {

/**
 * Try to do contiguous read.
 */
MGBASE_WARN_UNUSED_RESULT
inline bool try_read_async(const read_params& params) {
    return requester::get_instance().try_read_async(params);
}

/**
 * Try to do contiguous write.
 */
MGBASE_WARN_UNUSED_RESULT
inline bool try_write_async(const write_params& params) {
    return requester::get_instance().try_write_async(params);
}

} // namespace untyped

/**
 * Try to do contiguous read.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_read_async(const read_params<T>& params) {
    MGBASE_ASSERT(params.num_bytes == params.num_elems * mgbase::runtime_size_of<T>());
    return untyped::try_read_async(reinterpret_cast<const untyped::read_params&>(params));
}

/**
 * Try to do contiguous write.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_write_async(const write_params<T>& params) {
    MGBASE_ASSERT(params.num_bytes == params.num_elems * mgbase::runtime_size_of<T>());
    return untyped::try_write_async(reinterpret_cast<const untyped::write_params&>(params));
}

/**
 * Try to start atomic read.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_atomic_read_async(const atomic_read_params<T>& params) {
    return requester::get_instance().try_atomic_read_async(params);
}

/**
 * Try to start atomic write.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_atomic_write_async(const atomic_write_params<T>& params) {
    return requester::get_instance().try_atomic_write_async(params);
}

/**
 * Try to start remote compare-and-swap.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_compare_and_swap_async(const compare_and_swap_params<T>& params) {
    return requester::get_instance().try_compare_and_swap_async(params);
}

/**
 * Try to start remote fetch-and-add.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_fetch_and_add_async(const fetch_and_add_params<T>& params) {
    return requester::get_instance().try_fetch_and_add_async(params);
}


// For compatibility

namespace untyped {

/**
 * Try to do contiguous read.
 */
MGBASE_WARN_UNUSED_RESULT
inline bool try_read_async(
    const process_id_t          src_proc
,   const remote_address&       src_raddr
,   const local_address&        dest_laddr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    const read_params params = {
        src_proc
    ,   src_raddr
    ,   dest_laddr
    ,   size_in_bytes
    ,   on_complete
    };
    
    return try_read_async(params);
}

/**
 * Try to do contiguous write.
 */
MGBASE_WARN_UNUSED_RESULT
inline bool try_write_async(
    const process_id_t          dest_proc
,   const remote_address&       dest_raddr
,   const local_address&        src_laddr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    const write_params params = {
        dest_proc
    ,   dest_raddr
    ,   src_laddr
    ,   size_in_bytes
    ,   on_complete
    };
    
    return try_write_async(params);
}

} // namespace untyped

/**
 * Try to do contiguous read.
 */
template <typename Remote, typename Local>
MGBASE_WARN_UNUSED_RESULT
inline bool try_read_async(
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
    
    const untyped::read_params params = {
        src_proc
    ,   src_rptr.to_address()
    ,   dest_lptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    };
    
    return untyped::try_read_async(params);
}
/**
 * Try to do contiguous write.
 */
template <typename Remote, typename Local>
MGBASE_WARN_UNUSED_RESULT
inline bool try_write_async(
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
    
    const untyped::write_params params = {
        dest_proc
    ,   dest_rptr.to_address()
    ,   src_lptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<Remote>()
    ,   on_complete
    };
    
    return untyped::try_write_async(params);
}

/**
 * Try to start atomic read.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_atomic_read_async(
    const process_id_t          src_proc
,   const remote_ptr<const T>&  src_rptr
,   T* const                    dest_ptr
,   const mgbase::operation&    on_complete
) {
    const atomic_read_params<atomic_default_t> params = {
        src_proc
    ,   src_rptr
    ,   dest_ptr
    ,   on_complete
    };
    
    return try_atomic_read_async(params);
}

/**
 * Try to start atomic write.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_atomic_write_async(
    const process_id_t          dest_proc
,   const remote_ptr<T>&        dest_rptr
,   const T                     value
,   const mgbase::operation&    on_complete
) {
    const atomic_write_params<atomic_default_t> params = {
        dest_proc
    ,   dest_rptr
    ,   value
    ,   on_complete
    };
    
    return try_atomic_write_async(params);
}

/**
 * Try to start remote compare-and-swap.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_compare_and_swap_async(
    const process_id_t          target_proc
,   const remote_ptr<T>&        target_rptr
,   const T                     expected
,   const T                     desired
,   T* const                    result_ptr
,   const mgbase::operation&    on_complete
) {
    const compare_and_swap_params<atomic_default_t> params = {
        target_proc
    ,   target_rptr
    ,   expected
    ,   desired
    ,   result_ptr
    ,   on_complete
    };
    
    return try_compare_and_swap_async(params);
}

/**
 * Try to start remote fetch-and-add.
 */
template <typename T>
MGBASE_WARN_UNUSED_RESULT
inline bool try_fetch_and_add_async(
    const process_id_t          target_proc
,   const remote_ptr<T>&        target_rptr
,   const T                     value
,   T* const                    result_ptr
,   const mgbase::operation&    on_complete
) {
    const fetch_and_add_params<atomic_default_t> params = {
        target_proc
    ,   target_rptr
    ,   value
    ,   result_ptr
    ,   on_complete
    };
    
    return try_fetch_and_add_async(params);
}

} // namespace rma
} // namespace mgcom

