
#pragma once

#include <menps/medev/ucx/ucp/ucp.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct context_deleter
{
    void operator () (ucp_context*) const noexcept;
};

class context
    : public mefdn::unique_ptr<ucp_context, context_deleter>
{
    typedef mefdn::unique_ptr<ucp_context, context_deleter>  base;
    
public:
    context() noexcept = default;
    
    explicit context(ucp_context* const p)
        : base(p)
    { }
    
    context(const context&) = delete;
    context& operator = (const context&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(context, base)
    
    ucp_context_attr_t query();
};

context init(
    const ucp_params_t* params
,   const ucp_config_t* config
);

} // namespace ucp
} // namesapce ucx
} // namespace medev
} // namespace menps

