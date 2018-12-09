
#include <menps/medev/ucx/ucs/async_context.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucs {

void async_context_deleter::operator () (ucs_async_context_t* const p) const noexcept
{
    ucs_async_context_destroy(p);
}

async_context create_async_context(const ucs_async_mode_t mode)
{
    ucs_async_context_t* p = nullptr;
    
    const auto ret = ucs_async_context_create(mode, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucs_async_context_create() failed", ret);
    }
    
    return async_context(p);
}

} // namespace ucs
} // namespace ucx
} // namespace medev
} // namespace menps

