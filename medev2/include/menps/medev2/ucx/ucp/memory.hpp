
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>
#include <menps/medev2/ucx/ucx_error.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct memory_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type*    uf;
    ucp_context*        ctx;
    
    void operator () (ucp_mem* const p) const noexcept {
        uf->mem_unmap({ ctx, p });
    }
};

template <typename P>
class memory
    : public mefdn::unique_ptr<ucp_mem, memory_deleter<P>>
{
    using deleter_type = memory_deleter<P>;
    using base = mefdn::unique_ptr<ucp_mem, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    memory() noexcept = default;
    
    explicit memory(
        ucp_facade_type&    uf
    ,   ucp_context* const  ctx
    ,   ucp_mem* const      p
    )
        : base(p, deleter_type{ &uf, ctx })
    { }
    
    memory(const memory&) = delete;
    memory& operator = (const memory&) = delete;
    
    memory(memory&&) noexcept = default;
    memory& operator = (memory&&) noexcept = default;
    
    static memory map(
        ucp_facade_type&                    uf
    ,   ucp_context* const                  ctx
    ,   const ucp_mem_map_params_t* const   params
    ) {
        ucp_mem* p = nullptr;
        
        const auto ret = uf.mem_map({ ctx, params, &p });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_mem_map() failed", ret);
        }
        
        return memory(uf, ctx, p);
    }
    
    // for convenience
    static memory map_allocate(
        ucp_facade_type&    uf
    ,   ucp_context* const  ctx
    ,   const mefdn::size_t size_in_bytes
    ) {
        ucp_mem_map_params_t params = ucp_mem_map_params_t();
        
        params.field_mask =
            UCP_MEM_MAP_PARAM_FIELD_ADDRESS |
            UCP_MEM_MAP_PARAM_FIELD_LENGTH |
            UCP_MEM_MAP_PARAM_FIELD_FLAGS;
        
        params.address = nullptr;
        params.length = size_in_bytes;
        params.flags = UCP_MEM_MAP_ALLOCATE;
        
        return memory::map(uf, ctx, &params);
    }
    static memory map_register(
        ucp_facade_type&    uf
    ,   ucp_context* const  ctx
    ,   void* const         ptr
    ,   const mefdn::size_t size_in_bytes
    ) {
        ucp_mem_map_params_t params = ucp_mem_map_params_t();
        
        params.field_mask =
            UCP_MEM_MAP_PARAM_FIELD_ADDRESS |
            UCP_MEM_MAP_PARAM_FIELD_LENGTH |
            UCP_MEM_MAP_PARAM_FIELD_FLAGS;
        
        params.address = ptr;
        params.length = size_in_bytes;
        params.flags = 0;
        
        return memory::map(uf, ctx, &params);
    }
    
    ucp_mem_attr_t query(const ucp_mem_attr_field field_mask) const
    {
        ucp_mem_attr_t attr = ucp_mem_attr_t();
        attr.field_mask = field_mask;
        
        auto& uf = *this->get_deleter().uf;
        
        const auto ret = uf.mem_query({ this->get(), &attr });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_mem_query() failed", ret);
        }
        
        return attr;
    }
    
    void* get_address() const
    {
        const auto mem_attr = this->query(UCP_MEM_ATTR_FIELD_ADDRESS);
        return mem_attr.address;
    }
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

