
#include <menps/medev/ucx/ucp/rkey_buffer.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

void rkey_buffer_deleter::operator () (void* const p) const noexcept
{
    ucp_rkey_buffer_release(p);
}

rkey_buffer pack_rkey(ucp_context* const ctx, ucp_mem* const mem)
{
    void* p = nullptr;
    mefdn::size_t size = 0;
    
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
} // namespace medev
} // namespace menps

