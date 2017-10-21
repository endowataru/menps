
#pragma once

#include "sharer_space.hpp"
#include "basic_sharer_space_proxy.hpp"
#include <menps/mecom/structure/alltoall_ptr_group.hpp>
#include <menps/mecom/rpc/rpc_policy.hpp>

namespace menps {
namespace medsm {

struct sharer_space_proxy_policy
    : mecom::common_policy
    , mecom::rpc::rpc_policy
{
    typedef sharer_space::proxy         derived_type;
    typedef space_coherence_activater   interface_type;
    
    typedef sharer_space                space_type;
    
    typedef segment_id_t                segment_id_type;
    typedef page_id_t                   page_id_type;
    
    static const mecom::rpc::handler_id_t enable_flush_handler_id = 501;
    static const mecom::rpc::handler_id_t enable_diff_handler_id = 502;
};

class sharer_space::proxy
    : public basic_sharer_space_proxy<sharer_space_proxy_policy>
{
    typedef basic_sharer_space_proxy<sharer_space_proxy_policy> base;
    
public:
    explicit proxy(sharer_space& sp)
        : a2a_(&sp)
    { }
    
    proxy(const proxy&) = delete;
    proxy& operator = (const proxy&) = delete;
    
    proxy(proxy&&) noexcept = default;
    proxy& operator = (proxy&&) noexcept = default;
    
private:
    friend class basic_sharer_space_proxy<sharer_space_proxy_policy>;
    
    sharer_space* get_sharer_space_at_proc(const mecom::process_id_t proc) const noexcept
    {
        return this->a2a_.at_process(proc);
    }
    
    mecom::structure::alltoall_ptr_group<sharer_space> a2a_;
};

sharer_space::proxy sharer_space::make_proxy_collective()
{
    return sharer_space::proxy(*this);
}

} // namespace medsm
} // namespace menps

