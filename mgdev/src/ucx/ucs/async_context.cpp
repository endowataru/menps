
#include <mgdev/ucx/ucs/async_context.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucs {

void async_context_deleter::operator () (ucs_async_context_t* const p) const MGBASE_NOEXCEPT
{
    ucs_async_context_destroy(p);
}

async_context create_async_context(const ucs_async_mode_t mode)
{
    ucs_async_context_t* p = MGBASE_NULLPTR;
    
    const auto ret = ucs_async_context_create(mode, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucs_async_context_create() failed", ret);
    }
    
    return async_context(p);
}

} // namespace ucs
} // namespace ucx
} // namespace mgdev
