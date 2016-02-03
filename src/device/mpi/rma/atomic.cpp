
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
    process_id_t                                    proc
,   const remote_pointer<const atomic_default_t>&   remote_ptr
,   const local_pointer<atomic_default_t>&          local_ptr
,   const local_pointer<atomic_default_t>&          /*buf_ptr*/ // unused
,   const mgbase::operation&                        on_complete
) {
    return emulated_atomic<atomic_default_t>::try_read(
        proc
    ,   remote_ptr
    ,   local_ptr
    ,   on_complete
    );
}

bool try_remote_atomic_write_async(
    process_id_t                                    proc
,   const remote_pointer<atomic_default_t>&         remote_ptr
,   const local_pointer<const atomic_default_t>&    local_ptr
,   const local_pointer<atomic_default_t>&          /*buf_ptr*/ // unused
,   const mgbase::operation&                        on_complete
) {
    return emulated_atomic<atomic_default_t>::try_write(
        proc
    ,   remote_ptr
    ,   local_ptr
    ,   on_complete
    );
}

bool try_remote_compare_and_swap_async(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    expected_ptr
,   const local_pointer<const atomic_default_t>&    desired_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    return emulated_atomic<atomic_default_t>::try_compare_and_swap(
        target_proc
    ,   target_ptr
    ,   expected_ptr
    ,   desired_ptr
    ,   result_ptr
    ,   on_complete
    );
}

bool try_remote_fetch_and_add_async(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    value_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    return emulated_atomic<atomic_default_t>::try_fetch_and_add(
        target_proc
    ,   target_ptr
    ,   value_ptr
    ,   result_ptr
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

