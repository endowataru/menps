
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct iface_config_deleter
{
    void operator () (uct_iface_config_t*) const MGBASE_NOEXCEPT;
};

class iface_config
    : public mgbase::unique_ptr<uct_iface_config_t, iface_config_deleter>
{
    typedef mgbase::unique_ptr<uct_iface_config_t, iface_config_deleter>  base;
    
public:
    iface_config() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit iface_config(uct_iface_config_t* const p)
        : base(p)
    { }
    
    iface_config(const iface_config&) = delete;
    iface_config& operator = (const iface_config&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(iface_config, base)
};

iface_config read_iface_config(
    const char* tl_name
,   const char* env_prefix
,   const char* filename
);

} // namespace uct
} // namespace ucx
} // namespace mgdev

