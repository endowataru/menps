
#include "atomic.impl.hpp"

namespace mgcom {
namespace rma {

void initialize_atomic()
{
    emulated_atomic<atomic_default_t>::initialize();
}

void finalize_atomic()
{
    emulated_atomic<atomic_default_t>::finalize();
}

bool try_remote_atomic_read_async(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   atomic_default_t* const                     dest_ptr
,   const mgbase::operation&                    on_complete
) {
    return emulated_atomic<atomic_default_t>::try_read(
        src_proc
    ,   src_rptr
    ,   dest_ptr
    ,   on_complete
    );
}

bool try_remote_atomic_write_async(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const atomic_default_t                      value
,   const mgbase::operation&                    on_complete
) {
    return emulated_atomic<atomic_default_t>::try_write(
        dest_proc
    ,   dest_rptr
    ,   value
    ,   on_complete
    );
}

bool try_remote_compare_and_swap_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      expected_val
,   const atomic_default_t                      desired_val
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    return emulated_atomic<atomic_default_t>::try_compare_and_swap(
        target_proc
    ,   target_rptr
    ,   expected_val
    ,   desired_val
    ,   result_ptr
    ,   on_complete
    );
}

bool try_remote_fetch_and_add_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const atomic_default_t                      value
,   atomic_default_t* const                     result_ptr
,   const mgbase::operation&                    on_complete
) {
    return emulated_atomic<atomic_default_t>::try_fetch_and_add(
        target_proc
    ,   target_rptr
    ,   value
    ,   result_ptr
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

