
#include <mgdev/ucx/ucp/rkey_buffer.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void rkey_buffer_deleter::operator () (void* const p) const MGBASE_NOEXCEPT
{
    ucp_rkey_buffer_release(p);
}

rkey_buffer pack_rkey(ucp_context* const ctx, ucp_mem* const mem)
{
    void* p = MGBASE_NULLPTR;
    mgbase::size_t size = 0;
    
    const auto ret = ucp_rkey_pack(ctx, mem, &p, &size);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_rkey_pack() failed", ret);
    }
    
    return rkey_buffer(p, size);
}

remote_key rkey_buffer::unpack(ucp_ep* const ep)
{
    return unpack_rkey(ep, this->get());
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

