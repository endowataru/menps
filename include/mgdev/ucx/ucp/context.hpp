
#pragma once

#include <mgdev/ucx/ucp/ucp.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct context_deleter
{
    void operator () (ucp_context*) const MGBASE_NOEXCEPT;
};

class context
    : public mgbase::unique_ptr<ucp_context, context_deleter>
{
    typedef mgbase::unique_ptr<ucp_context, context_deleter>  base;
    
public:
    context() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit context(ucp_context* const p)
        : base(p)
    { }
    
    context(const context&) = delete;
    context& operator = (const context&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(context, base)
    
    ucp_context_attr_t query();
};

context init(
    const ucp_params_t* params
,   const ucp_config_t* config
);

} // namespace ucp
} // namesapce ucx
} // namespace mgdev

