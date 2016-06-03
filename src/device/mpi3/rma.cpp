
#include "rma.hpp"
#include "rma.impl.hpp"
#include "mpi3_call.hpp"
#include "common/rma/region_allocator.hpp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

mpi3_rma g_impl;

} // unnamed namespace

void initialize()
{
    g_impl.initialize();
}
void finalize()
{
    g_impl.finalize();
}

MPI_Win get_win() MGBASE_NOEXCEPT
{
    return g_impl.get_win();
}


namespace untyped {

local_region register_region(
    void*   local_ptr
,   index_t size_in_bytes
) {
    const MPI_Aint addr = g_impl.attach(local_ptr, static_cast<MPI_Aint>(size_in_bytes));
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

bool try_remote_read_async(
    const process_id_t          src_proc
,   const remote_address&       src_raddr
,   const local_address&        dest_laddr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return mpi3::try_get(
        to_raw_pointer(dest_laddr)
    ,   static_cast<int>(src_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(src_raddr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

bool try_remote_write_async(
    const process_id_t          dest_proc
,   const remote_address&       dest_raddr
,   const local_address&        src_laddr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return mpi3::try_put(
        to_raw_pointer(src_laddr)
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(dest_raddr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

} // namespace untyped

bool try_remote_atomic_read_async(
    const process_id_t                          src_proc
,   const remote_ptr<const atomic_default_t>&   src_rptr
,   const local_ptr<atomic_default_t>&          dest_lptr
,   const local_ptr<atomic_default_t>&          buf_lptr
,   const mgbase::operation&                    on_complete
) {
    return mpi3::try_fetch_and_op(
        to_raw_pointer(buf_lptr)
    ,   to_raw_pointer(dest_lptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(src_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(src_rptr))
    ,   MPI_NO_OP
    ,   on_complete
    );
}

bool try_remote_atomic_write_async(
    const process_id_t                          dest_proc
,   const remote_ptr<atomic_default_t>&         dest_rptr
,   const local_ptr<const atomic_default_t>&    src_lptr
,   const local_ptr<atomic_default_t>&          buf_lptr
,   const mgbase::operation&                    on_complete
) {
    return mpi3::try_fetch_and_op(
        to_raw_pointer(src_lptr)
    ,   to_raw_pointer(buf_lptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(dest_rptr))
    ,   MPI_REPLACE
    ,   on_complete
    );
}

bool try_remote_compare_and_swap_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const local_ptr<const atomic_default_t>&    expected_lptr
,   const local_ptr<const atomic_default_t>&    desired_lptr
,   const local_ptr<atomic_default_t>&          result_lptr
,   const mgbase::operation&                    on_complete
) {
    return mpi3::try_compare_and_swap(
        to_raw_pointer(expected_lptr)
    ,   to_raw_pointer(desired_lptr)
    ,   to_raw_pointer(result_lptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(target_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(target_rptr))
    ,   on_complete
    );
}

bool try_remote_fetch_and_add_async(
    const process_id_t                          target_proc
,   const remote_ptr<atomic_default_t>&         target_rptr
,   const local_ptr<const atomic_default_t>&    value_lptr
,   const local_ptr<atomic_default_t>&          result_lptr
,   const mgbase::operation&                    on_complete
) {
    return mpi3::try_fetch_and_op(
        to_raw_pointer(value_lptr)
    ,   to_raw_pointer(result_lptr)
    ,   MPI_UINT64_T
    ,   static_cast<int>(target_proc)
    ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(target_rptr))
    ,   MPI_SUM
    ,   on_complete
    );
}

} // namespace rma
} // namespace mgcom

