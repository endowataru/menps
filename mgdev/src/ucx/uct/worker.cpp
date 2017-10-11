
#include <mgdev/ucx/uct/worker.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

void worker_deleter::operator () (uct_worker* const p) const MGBASE_NOEXCEPT
{
    uct_worker_destroy(p);
}

worker create_worker(
    ucs_async_context_t* const  async
,   ucs_thread_mode_t const     thread_mode
) {
    uct_worker* p = MGBASE_NULLPTR;
    
    const auto ret = uct_worker_create(async, thread_mode, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_worker_create() failed", ret);
    }
    
    return worker(p);
}

} // namespace uct
} // namespace ucx
} // namespace mgdev

