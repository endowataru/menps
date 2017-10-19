
#include <menps/medev/ucx/ucp/worker_address.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

void worker_address_deleter::operator () (ucp_address_t* const p) const noexcept
{
    ucp_worker_release_address(this->wk, p);
}

worker_address get_worker_address(ucp_worker* const wk)
{
    ucp_address_t* p = nullptr;
    mefdn::size_t size = 0;
    
    const auto ret = ucp_worker_get_address(wk, &p, &size);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_worker_get_address() failed", ret);
    }
    
    return worker_address(wk, p, size);
}

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

