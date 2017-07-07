
#include <mgdev/ucx/uct/tl_resource_list.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace uct {

void tl_resource_list_deleter::operator () (uct_tl_resource_desc_t* const p) const MGBASE_NOEXCEPT
{
    uct_release_tl_resource_list(p);
}

tl_resource_list query_tl_resources(uct_md* const md)
{
    uct_tl_resource_desc_t* p = MGBASE_NULLPTR;
    unsigned int num = 0;
    
    const auto ret = uct_md_query_tl_resources(md, &p, &num);
    if (ret != UCS_OK) {
        throw ucx_error("uct_query_tl_resources() failed", ret);
    }
    
    return tl_resource_list(p, num);
}

} // namespace uct
} // namespace ucx
} // namespace mgdev

