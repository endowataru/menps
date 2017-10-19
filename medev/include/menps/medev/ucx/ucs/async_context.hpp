
#pragma once

extern "C" {

#include <ucs/type/status.h>
#include <ucs/async/async_fwd.h>

} // extern "C"

#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucs {

struct async_context_deleter
{
    void operator () (ucs_async_context_t*) const noexcept;
};

class async_context
    : public mefdn::unique_ptr<ucs_async_context_t, async_context_deleter>
{
    typedef mefdn::unique_ptr<ucs_async_context_t, async_context_deleter>  base;
    
public:
    async_context() noexcept = default;
    
    explicit async_context(ucs_async_context_t* const p)
        : base(p)
    { }
    
    async_context(const async_context&) = delete;
    async_context& operator = (const async_context&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(async_context, base)
};

async_context create_async_context(ucs_async_mode_t mode);

} // namespace ucs
} // namespace ucx
} // namespace medev
} // namespace menps

