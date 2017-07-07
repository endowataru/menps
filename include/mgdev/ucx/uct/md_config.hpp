
#pragma once

#include <mgdev/ucx/uct/uct.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

struct md_config_deleter
{
    void operator() (uct_md_config_t*) const MGBASE_NOEXCEPT;
};

class md_config
    : public mgbase::unique_ptr<uct_md_config_t, md_config_deleter>
{
    typedef mgbase::unique_ptr<uct_md_config_t, md_config_deleter>  base;
    
public:
    md_config() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit md_config(uct_md_config_t* const md)
        : base(md)
    { }
    
    md_config(const md_config&) = delete;
    md_config& operator = (const md_config&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(md_config, base)
};

md_config read_md_config(const char* md_name);

} // namespace uct
} // namesapce ucx
} // namespace mgdev

