
#include <mgdev/ucx/ucp/worker_address.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void worker_address_deleter::operator () (ucp_address_t* const p) const MGBASE_NOEXCEPT
{
    ucp_worker_release_address(this->wk, p);
}

worker_address get_worker_address(ucp_worker* const wk)
{
    ucp_address_t* p = MGBASE_NULLPTR;
    mgbase::size_t size = 0;
    
    const auto ret = ucp_worker_get_address(wk, &p, &size);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_worker_get_address() failed", ret);
    }
    
    return worker_address(wk, p, size);
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

