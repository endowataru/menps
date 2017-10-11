
#include <mgdev/ucx/ucp/memory.hpp>
#include <mgdev/ucx/ucx_error.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

void memory_deleter::operator () (ucp_mem* const p) const MGBASE_NOEXCEPT
{
    ucp_mem_unmap(this->ctx, p);
}

memory map_memory(
    ucp_context* const                  ctx
,   const ucp_mem_map_params_t* const   params
) {
    ucp_mem* p = MGBASE_NULLPTR;
    
    const auto ret = ucp_mem_map(ctx, params, &p);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_mem_map() failed", ret);
    }
    
    return memory(ctx, p);
}

ucp_mem_attr_t memory::query(const ucp_mem_attr_field field_mask)
{
    ucp_mem_attr_t attr = ucp_mem_attr_t();
    attr.field_mask = field_mask;
    
    const auto ret = ucp_mem_query(this->get(), &attr);
    if (ret != UCS_OK) {
        throw ucx_error("ucp_mem_query() failed", ret);
    }
    
    return attr;
}

rkey_buffer memory::pack_rkey()
{
    return ucp::pack_rkey(this->get_deleter().ctx, this->get());
}

} // namespace ucp
} // namespace ucx
} // namespace mgdev

