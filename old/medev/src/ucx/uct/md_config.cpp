
#include <menps/medev/ucx/uct/md_config.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void md_config_deleter::operator () (uct_md_config_t* const p) const noexcept
{
    uct_config_release(p);
}

md_config read_md_config(const char* const md_name)
{
    uct_md_config_t* p = nullptr;
    
    const auto ret = uct_md_config_read(md_name, nullptr, nullptr, &p);
    if (ret != UCS_OK) {
        throw ucx_error("uct_md_config_read() failed", ret);
    }
    
    return md_config(p);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

