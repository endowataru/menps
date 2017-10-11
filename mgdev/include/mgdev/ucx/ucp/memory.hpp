
#pragma once

#include <mgdev/ucx/ucp/rkey_buffer.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct memory_deleter
{
    ucp_context* ctx;
    
    void operator () (ucp_mem*) const MGBASE_NOEXCEPT;
};

class memory
    : public mgbase::unique_ptr<ucp_mem, memory_deleter>
{
    typedef mgbase::unique_ptr<ucp_mem, memory_deleter>  base;
    
public:
    memory() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit memory(ucp_context* ctx, ucp_mem* const p)
        : base(p, memory_deleter{ctx})
    { }
    
    memory(const memory&) = delete;
    memory& operator = (const memory&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(memory, base)
    
    ucp_mem_attr_t query(ucp_mem_attr_field field_mask);
    
    rkey_buffer pack_rkey();
};

memory map_memory(
    ucp_context*                ctx
,   const ucp_mem_map_params_t* params
);

} // namespace ucp
} // namespace ucx
} // namespace mgdev
