
#pragma once

#include <menps/medev2/ucx/ucs/ucs.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucs {

struct async_context_deleter
{
    void operator () (ucs_async_context_t* const p) const noexcept
    {
        ucs_async_context_destroy(p);
    }
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
    
    async_context(async_context&&) noexcept = default;
    async_context& operator = (async_context&&) noexcept = default;
    
    static async_context create(ucs_async_mode_t mode)
    {
        ucs_async_context_t* p = nullptr;
        
        const auto ret = ucs_async_context_create(mode, &p);
        if (ret != UCS_OK) {
            throw ucx_error("ucs_async_context_create() failed", ret);
        }
        
        return async_context(p);
    }
};

} // namespace ucs
} // namespace ucx
} // namespace medev2
} // namespace menps

