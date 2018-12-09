
#include <menps/medev/ucx/uct/worker.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void worker_deleter::operator () (uct_worker* const p) const noexcept
{
    uct_worker_destroy(p);
}

worker create_worker(
    ucs_async_context_t* const  async
,   ucs_thread_mode_t const     thread_mode
) {
    uct_worker* p = nullptr;
    
    const auto ret = uct_worker_create(async, thread_mode, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_worker_create() failed", ret);
    }
    
    return worker(p);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

