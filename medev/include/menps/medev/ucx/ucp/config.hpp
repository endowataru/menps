
#pragma once

#include <menps/medev/ucx/ucp/ucp.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct config_deleter
{
    void operator () (ucp_config*) const noexcept;
};

class config
    : public mefdn::unique_ptr<ucp_config, config_deleter>
{
    typedef mefdn::unique_ptr<ucp_config, config_deleter>  base;
    
public:
    config() noexcept = default;
    
    explicit config(ucp_config* const p)
        : base(p)
    { }
   
    config(const config&) = delete;
    config& operator = (const config&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(config, base)
};

config read_config(
    const char* env_prefix
,   const char* filename
);

} // namespace ucp
} // namesapce ucx
} // namespace medev
} // namespace menps

