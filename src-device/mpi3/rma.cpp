
#include "rma.ipp"

namespace mgcom {

namespace rma {

namespace {

impl g_impl;

}

void initialize() {
    g_impl.initialize();
}

void finalize() {
    g_impl.finalize();
}


local_region register_region(
    void*   local_pointer
,   index_t size_in_bytes
) {
    const MPI_Aint addr = g_impl.attach(local_pointer, static_cast<MPI_Aint>(size_in_bytes));
    return make_local_region(make_region_key(reinterpret_cast<void*>(addr), 0), 0);
}

void deregister_region(const local_region& region)
{
    g_impl.detach(to_pointer(region));
}

remote_region use_remote_region(
    process_id_t      /*proc_id*/
,   const region_key& key
) {
    // Do nothing on MPI-3
    return make_remote_region(key, 0 /* unused */);
}

bool try_read_async(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
)
{
    return g_impl.try_get(
        to_pointer(local_addr)
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_pointer(remote_addr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

bool try_write_async(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
)
{
    return g_impl.try_put(
        to_pointer(local_addr)
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_pointer(remote_addr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

bool try_compare_and_swap_64_async(
    const remote_address&   remote_addr
,   const mgbase::uint64_t* expected
,   const mgbase::uint64_t* desired
,   mgbase::uint64_t*       result
,   process_id_t            dest_proc
,   local_notifier          on_complete
)
{
    return g_impl.try_compare_and_swap(
        expected
    ,   desired
    ,   result
    ,   MPI_UINT64_T
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_pointer(remote_addr))
    ,   on_complete
    );
}

void poll() {
    g_impl.flush();
}

}

}

