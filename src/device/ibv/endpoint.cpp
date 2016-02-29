
#include "endpoint.hpp"
#include "poll_thread.hpp"
#include <mgcom/rma/try_rma.hpp>
#include "common/rma/region_allocator.hpp"

namespace mgcom {

namespace ibv {

endpoint g_endpoint;

poll_thread g_poll;

bool try_get_new_wr_id(const mgbase::operation& on_complete, mgbase::uint64_t* const wr_id_result)
{
    return g_poll.try_complete(on_complete, wr_id_result);
}

} // namespace ibv

namespace rma {

void initialize()
{
    ibv::g_endpoint.initialize();
    ibv::g_poll.start(ibv::g_endpoint.get_cq());
    
    // The initialization of the allocator will use register_region()
    initialize_allocator();
}

void finalize()
{
    finalize_allocator();
    
    ibv::g_poll.stop();
    ibv::g_endpoint.finalize();
}

namespace untyped {

namespace /*unnamed*/ {

MGBASE_ALWAYS_INLINE mgbase::uint64_t to_raddr(const remote_address& addr) MGBASE_NOEXCEPT {
    return reinterpret_cast<mgbase::uint64_t>(addr.region.key.pointer) + addr.offset;
}

MGBASE_ALWAYS_INLINE mgbase::uint32_t to_rkey(const remote_address& addr) MGBASE_NOEXCEPT {
    return addr.region.key.info;
}

MGBASE_ALWAYS_INLINE ibv_mr* to_mr(const local_region& region) MGBASE_NOEXCEPT {
    return reinterpret_cast<ibv_mr*>(region.info);
}

MGBASE_ALWAYS_INLINE mgbase::uint64_t to_laddr(const local_address& addr) MGBASE_NOEXCEPT {
    const ibv_mr* const mr = to_mr(addr.region);
    return reinterpret_cast<mgbase::uint64_t>(mr->addr) + addr.offset;
}

MGBASE_ALWAYS_INLINE mgbase::uint32_t to_lkey(const local_address& addr) MGBASE_NOEXCEPT {
    const ibv_mr* const mr = to_mr(addr.region);
    return mr->lkey;
}

} // unnamed namespace

local_region register_region(
    void* const     buf
,   const index_t   size_in_bytes
) {
    ibv_mr* const mr = ibv::g_endpoint.register_memory(buf, size_in_bytes);
    
    const region_key key = { buf, mr->lkey };
    const local_region reg = { key, reinterpret_cast<mgbase::uint64_t>(mr) };
    return reg;
}

void deregister_region(const local_region& region)
{
    ibv_mr* const mr = to_mr(region);
    ibv::g_endpoint.deregister_memory(mr);
}

remote_region use_remote_region(
    const process_id_t  //proc_id
,   const region_key&   key
) {
    remote_region region;
    region.key = key;
    return region;
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_read_async(
    const process_id_t          src_proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    mgbase::uint64_t wr_id;
    if (MGBASE_UNLIKELY(!ibv::try_get_new_wr_id(on_complete, &wr_id)))
        return false;
    
    const bool ret = ibv::g_endpoint.try_read_async(
        wr_id
    ,   src_proc
    ,   to_raddr(remote_addr)
    ,   to_rkey(remote_addr)
    ,   to_laddr(local_addr)
    ,   to_lkey(local_addr)
    ,   size_in_bytes
    );
    
    if (MGBASE_LIKELY(ret))
        return true;
    else {
        ibv::g_poll.failed(wr_id);
        return false;
    }
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_write_async(
    const process_id_t          dest_proc
,   const remote_address&       remote_addr
,   const local_address&        local_addr
,   const index_t               size_in_bytes
,   const mgbase::operation&    on_complete
) {
    mgbase::uint64_t wr_id;
    if (MGBASE_UNLIKELY(!ibv::try_get_new_wr_id(on_complete, &wr_id)))
        return false;
    
    const bool ret = ibv::g_endpoint.try_write_async(
        wr_id
    ,   dest_proc
    ,   to_raddr(remote_addr)
    ,   to_rkey(remote_addr)
    ,   to_laddr(local_addr)
    ,   to_lkey(local_addr)
    ,   size_in_bytes
    );
    
    if (MGBASE_LIKELY(ret))
        return true;
    else {
        ibv::g_poll.failed(wr_id);
        return false;
    }
}

} // namespace untyped

MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_read_async(
    const process_id_t                              target_proc
,   const remote_pointer<const atomic_default_t>&   remote_ptr
,   const local_pointer<atomic_default_t>&          local_ptr
,   const local_pointer<atomic_default_t>&          //buf_ptr
,   const mgbase::operation&                        on_complete
) {
    // TODO: Is this correct?
    return untyped::try_remote_read_async(
        target_proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   sizeof(atomic_default_t)
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_atomic_write_async(
    const process_id_t                              target_proc
,   const remote_pointer<atomic_default_t>&         remote_ptr
,   const local_pointer<const atomic_default_t>&    local_ptr
,   const local_pointer<atomic_default_t>&          //buf_ptr
,   const mgbase::operation&                        on_complete
) {
    // TODO: Is this correct?
    return untyped::try_remote_write_async(
        target_proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   sizeof(atomic_default_t)
    ,   on_complete
    );
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_compare_and_swap_async(
    const process_id_t                              target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    expected_ptr
,   const local_pointer<const atomic_default_t>&    desired_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    mgbase::uint64_t wr_id;
    if (MGBASE_UNLIKELY(!ibv::try_get_new_wr_id(on_complete, &wr_id)))
        return false;
    
    const untyped::remote_address target_raddr = target_ptr.to_address();
    const untyped::local_address  result_laddr = result_ptr.to_address();
    
    const bool ret = ibv::g_endpoint.try_compare_and_swap_async(
        wr_id
    ,   target_proc
    ,   untyped::to_raddr(target_raddr)
    ,   untyped::to_rkey(target_raddr)
    ,   untyped::to_laddr(result_laddr)
    ,   untyped::to_lkey(result_laddr)
    ,   *expected_ptr
    ,   *desired_ptr
    );
    
    if (MGBASE_LIKELY(ret))
        return true;
    else {
        ibv::g_poll.failed(wr_id);
        return false;
    }
}

MGBASE_WARN_UNUSED_RESULT bool try_remote_fetch_and_add_async(
    const process_id_t                              target_proc
,   const remote_pointer<atomic_default_t>&         target_ptr
,   const local_pointer<const atomic_default_t>&    value_ptr
,   const local_pointer<atomic_default_t>&          result_ptr
,   const mgbase::operation&                        on_complete
) {
    mgbase::uint64_t wr_id;
    if (MGBASE_UNLIKELY(!ibv::try_get_new_wr_id(on_complete, &wr_id)))
        return false;
    
    const untyped::remote_address target_raddr = target_ptr.to_address();
    const untyped::local_address  result_laddr = result_ptr.to_address();
    
    const bool ret = ibv::g_endpoint.try_fetch_and_add_async(
        wr_id
    ,   target_proc
    ,   untyped::to_raddr(target_raddr)
    ,   untyped::to_rkey(target_raddr)
    ,   untyped::to_laddr(result_laddr)
    ,   untyped::to_lkey(result_laddr)
    ,   *value_ptr
    );
    
    if (MGBASE_LIKELY(ret))
        return true;
    else {
        ibv::g_poll.failed(wr_id);
        return false;
    }
}

} // namespace rma
} // namespace mgcom

