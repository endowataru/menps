
#pragma once

#include <mgdev/ucx/ucp/ucp.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct config_deleter
{
    void operator () (ucp_config*) const MGBASE_NOEXCEPT;
};

class config
    : public mgbase::unique_ptr<ucp_config, config_deleter>
{
    typedef mgbase::unique_ptr<ucp_config, config_deleter>  base;
    
public:
    config() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit config(ucp_config* const p)
        : base(p)
    { }
   
    config(const config&) = delete;
    config& operator = (const config&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(config, base)
};

config read_config(
    const char* env_prefix
,   const char* filename
);

} // namespace ucp
} // namesapce ucx
} // namespace mgdev

