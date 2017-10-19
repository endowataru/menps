
#pragma once

#include <menps/medev/ucx/uct/uct.hpp>
#include <menps/mefdn/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

struct md_config_deleter
{
    void operator() (uct_md_config_t*) const noexcept;
};

class md_config
    : public mefdn::unique_ptr<uct_md_config_t, md_config_deleter>
{
    typedef mefdn::unique_ptr<uct_md_config_t, md_config_deleter>  base;
    
public:
    md_config() noexcept = default;
    
    explicit md_config(uct_md_config_t* const md)
        : base(md)
    { }
    
    md_config(const md_config&) = delete;
    md_config& operator = (const md_config&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(md_config, base)
};

md_config read_md_config(const char* md_name);

} // namespace uct
} // namesapce ucx
} // namespace medev
} // namespace menps

