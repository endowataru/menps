
#pragma once

#include <menps/medev/ucx/ucp/rkey_buffer.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct memory_deleter
{
    ucp_context* ctx;
    
    void operator () (ucp_mem*) const noexcept;
};

class memory
    : public mefdn::unique_ptr<ucp_mem, memory_deleter>
{
    typedef mefdn::unique_ptr<ucp_mem, memory_deleter>  base;
    
public:
    memory() noexcept = default;
    
    explicit memory(ucp_context* ctx, ucp_mem* const p)
        : base(p, memory_deleter{ctx})
    { }
    
    memory(const memory&) = delete;
    memory& operator = (const memory&) = delete;
    
    memory(memory&&) noexcept = default;
    memory& operator = (memory&&) noexcept = default;
    
    ucp_mem_attr_t query(ucp_mem_attr_field field_mask);
    
    rkey_buffer pack_rkey();
};

memory map_memory(
    ucp_context*                ctx
,   const ucp_mem_map_params_t* params
);

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps
