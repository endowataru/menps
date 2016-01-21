
#include "rma.ipp"
#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

impl g_impl;

} // unnamed namespace

void initialize() {
    g_impl.initialize();
    initialize_allocator();
}

void finalize() {
    finalize_allocator();
    g_impl.finalize();
}

namespace untyped {

local_region register_region(
    void*   local_pointer
,   index_t size_in_bytes
) {
    const MPI_Aint addr = g_impl.attach(local_pointer, static_cast<MPI_Aint>(size_in_bytes));
    return make_local_region(make_region_key(reinterpret_cast<void*>(addr), 0), 0);
}

void deregister_region(const local_region& region)
{
    g_impl.detach(to_raw_pointer(region));
}

remote_region use_remote_region(
    process_id_t      /*proc_id*/
,   const region_key& key
) {
    // Do nothing on MPI-3
    return make_remote_region(key, 0 /* unused */);
}


bool try_remote_read(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return g_impl.try_get(
        to_raw_pointer(local_addr)
    ,   static_cast<int>(proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(remote_addr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

bool try_remote_write(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return g_impl.try_put(
        to_raw_pointer(local_addr)
    ,   static_cast<int>(proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(remote_addr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

} // namespace untyped

bool try_remote_atomic_read(
    process_id_t                                    proc
,   const remote_pointer<const atomic_default_t>&   remote_ptr
,   const local_pointer<atomic_default_t>&          local_ptr
,   const local_pointer<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                        on_complete
) {
    return g_impl.try_fetch_and_op(
        to_raw_pointer(buf_ptr)
    ,   to_raw_pointer(local_ptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(remote_ptr))
    ,   MPI_NO_OP
    ,   on_complete
    );
}

bool try_remote_atomic_write(
    process_id_t                                    proc
,   const remote_pointer<atomic_default_t>&         remote_ptr
,   const local_pointer<const atomic_default_t>&    local_ptr
,   const local_pointer<atomic_default_t>&          buf_ptr
,   const mgbase::operation&                        on_complete
) {
    return g_impl.try_fetch_and_op(
        to_raw_pointer(local_ptr)
    ,   to_raw_pointer(buf_ptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(remote_ptr))
    ,   MPI_REPLACE
    ,   on_complete
    );
}

bool try_remote_compare_and_swap(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    expected_ptr
,   const local_pointer<const atomic_default_t>&    desired_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    return g_impl.try_compare_and_swap(
        to_raw_pointer(expected_ptr)
    ,   to_raw_pointer(desired_ptr)
    ,   to_raw_pointer(result_ptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(target_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(target_ptr))
    ,   on_complete
    );
}

bool try_remote_fetch_and_add(
    process_id_t                                    target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    value_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
)
{
    return g_impl.try_fetch_and_op(
        to_raw_pointer(value_ptr)
    ,   to_raw_pointer(result_ptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(target_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(target_ptr))
    ,   MPI_SUM
    ,   on_complete
    );
}

void poll() {
    g_impl.flush();
}

} // namespace rma
} // namespace mgcom

