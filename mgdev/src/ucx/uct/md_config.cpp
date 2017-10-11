
#include <mgdev/ucx/uct/md_config.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

void md_config_deleter::operator () (uct_md_config_t* const p) const MGBASE_NOEXCEPT
{
    uct_config_release(p);
}

md_config read_md_config(const char* const md_name)
{
    uct_md_config_t* p = MGBASE_NULLPTR;
    
    const auto ret = uct_md_config_read(md_name, MGBASE_NULLPTR, MGBASE_NULLPTR, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_md_config_read() failed", ret);
    }
    
    return md_config(p);
}

} // namespace uct
} // namespace ucx
} // namespace mgdev

