
#include <mgdev/ucx/ucp/remote_key.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void remote_key_deleter::operator () (ucp_rkey* const p) const MGBASE_NOEXCEPT
{
    ucp_rkey_destroy(p);
}

remote_key unpack_rkey(
    ucp_ep* const   ep
,   void* const     rkey_buf
) {
    ucp_rkey* p = MGBASE_NULLPTR;
    
    const auto ret = ucp_ep_rkey_unpack(ep, rkey_buf, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_ep_rkey_unpack() failed", ret);
    }
    
    return remote_key(p);
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

