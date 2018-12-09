
#include <menps/medev/ucx/uct/md_resource_list.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void md_resource_list_deleter::operator () (uct_md_resource_desc_t* const p) const noexcept
{
    uct_release_md_resource_list(p);
}

md_resource_list query_md_resources()
{
    uct_md_resource_desc_t* p = nullptr;
    unsigned int num = 0;
    
    const auto ret = uct_query_md_resources(&p, &num);
    if (ret != UCS_OK) {
        throw ucx_error("uct_query_md_resources() failed", ret);
    }
    
    return md_resource_list(p, num);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

