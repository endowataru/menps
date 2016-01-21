
#include "rma.ipp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

impl g_impl;

} // unnamed namespace

void initialize_contiguous() {
    g_impl.initialize();
}
void finalize_contiguous() {
    g_impl.finalize();
}

void poll() {
    g_impl.poll();
}

namespace untyped {

local_region register_region(
    void*   local_pointer
,   index_t size_in_bytes
) {
    mgbase::uint64_t address;
    const int memid = g_impl.register_memory(local_pointer, size_in_bytes, &address);
    
    return make_local_region(
        make_region_key(
            local_pointer
        ,   static_cast<mgbase::uint64_t>(memid)
        )
    ,   address
    );
}

remote_region use_remote_region(
    process_id_t      proc_id
,   const region_key& key
) {
    const mgbase::uint64_t raddr =
        g_impl.get_remote_addr(static_cast<int>(proc_id), static_cast<int>(key.info));
    
    return make_remote_region(key, raddr);
}

void deregister_region(const local_region& region)
{
    g_impl.deregister_memory(static_cast<int>(region.key.info));
}

namespace /*unnamed*/ {

inline mgbase::uint64_t get_absolute_address(const local_address& addr) MGBASE_NOEXCEPT {
    const mgbase::uint64_t laddr = addr.region.info;
    return laddr + addr.offset;
}

inline mgbase::uint64_t get_absolute_address(const remote_address& addr) MGBASE_NOEXCEPT {
    const mgbase::uint64_t raddr = addr.region.info;
    return raddr + addr.offset;
}

} // unnamed namespace

bool try_remote_read_extra(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
,   int                         flags
) {
    return g_impl.try_get(
        static_cast<int>(proc)
    ,   get_absolute_address(local_addr)
    ,   get_absolute_address(remote_addr)
    ,   size_in_bytes
    ,   on_complete
    ,   flags
    );
}

bool try_remote_read(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return try_remote_read_extra(
        proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   on_complete
    ,   0
    );
}

bool try_remote_write_extra(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
,   int                         flags
) {
    return g_impl.try_put(
        static_cast<int>(proc)
    ,   get_absolute_address(local_addr)
    ,   get_absolute_address(remote_addr)
    ,   size_in_bytes
    ,   on_complete
    ,   flags
    );
}

bool try_remote_write(
    process_id_t                proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   index_t                     size_in_bytes
,   const mgbase::operation&    on_complete
) {
    return try_remote_write_extra(
        proc
    ,   remote_addr
    ,   local_addr
    ,   size_in_bytes
    ,   on_complete
    ,   0
    );
}

} // namespace untyped

} // namespace rma
} // namespace mgcom

