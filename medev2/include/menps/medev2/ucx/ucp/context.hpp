
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>
#include <menps/medev2/ucx/ucx_error.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct context_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type* uf;
    
    void operator () (ucp_context* const ep) const noexcept {
        this->uf->cleanup({ ep });
    }
};

template <typename P>
class context
    : public mefdn::unique_ptr<ucp_context, context_deleter<P>>
{
    using deleter_type = context_deleter<P>;
    using base = mefdn::unique_ptr<ucp_context, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    context() noexcept = default;
    
    explicit context(ucp_facade_type& uf, ucp_context* const p)
        : base(p, deleter_type{ &uf })
    { }
    
    context(const context&) = delete;
    context& operator = (const context&) = delete;
    
    context(context&&) noexcept = default;
    context& operator = (context&&) noexcept = default;
    
    static context init(
        ucp_facade_type&            uf
    ,   const ucp_params_t* const   params
    ,   const ucp_config_t* const   config
    ) {
        ucp_context* ctx = nullptr;
        
        const auto ret = uf.init({ params, config, &ctx });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_init() failed", ret);
        }
        
        return context(uf, ctx);
    }
};

} // namespace ucp
} // namesapce ucx
} // namespace medev2
} // namespace menps

