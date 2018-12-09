
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct iface_config_deleter
{
    void operator () (uct_iface_config_t*) const noexcept;
};

class iface_config
    : public mefdn::unique_ptr<uct_iface_config_t, iface_config_deleter>
{
    typedef mefdn::unique_ptr<uct_iface_config_t, iface_config_deleter>  base;
    
public:
    iface_config() noexcept = default;
    
    explicit iface_config(uct_iface_config_t* const p)
        : base(p)
    { }
    
    iface_config(const iface_config&) = delete;
    iface_config& operator = (const iface_config&) = delete;
    
    iface_config(iface_config&&) noexcept = default;
    iface_config& operator = (iface_config&&) noexcept = default;
};

iface_config read_iface_config(
    uct_md_t*   md
,   const char* tl_name
,   const char* env_prefix
,   const char* filename
);

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

