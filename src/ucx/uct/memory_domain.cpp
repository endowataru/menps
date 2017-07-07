
#include <mgdev/ucx/uct/memory_domain.hpp>
#include <mgdev/ucx/uct/md_config.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

void memory_domain_deleter::operator () (uct_md* const md) const MGBASE_NOEXCEPT
{
    uct_md_close(md);
}

memory_domain open_memory_domain(const char* const md_name, uct_md_config_t* const conf)
{
    uct_md* md = MGBASE_NULLPTR;
    
    const auto ret = uct_md_open(md_name, conf, &md);
    if (ret != UCS_OK) {
        throw ucx_error("uct_md_open() failed", ret);
    }
    
    return memory_domain(md);
}

memory_domain open_memory_domain(uct_md_resource_desc_t* const md_desc)
{
    const auto conf = read_md_config(md_desc->md_name);
    return open_memory_domain(md_desc->md_name, conf.get());
}

} // namespace uct
} // namespace ucx
} // namespace mgdev

