
#include "rma.hpp"
#include "rma.impl.hpp"
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


} // namespace untyped

} // namespace rma
} // namespace mgcom

