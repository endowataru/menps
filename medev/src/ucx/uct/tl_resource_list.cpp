
#include <menps/medev/ucx/uct/tl_resource_list.hpp>
#include <menps/medev/ucx/ucx_error.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace uct {

void tl_resource_list_deleter::operator () (uct_tl_resource_desc_t* const p) const noexcept
{
    uct_release_tl_resource_list(p);
}

tl_resource_list query_tl_resources(uct_md* const md)
{
    uct_tl_resource_desc_t* p = nullptr;
    unsigned int num = 0;
    
    const auto ret = uct_md_query_tl_resources(md, &p, &num);
    if (ret != UCS_OK) {
        throw ucx_error("uct_query_tl_resources() failed", ret);
    }
    
    return tl_resource_list(p, num);
}

} // namespace uct
} // namespace ucx
} // namespace medev
} // namespace menps

