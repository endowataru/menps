
#pragma once

#include "protector_space.hpp"
#include "basic_protector_space_proxy.hpp"
#include <menps/mecom/structure/alltoall_ptr_group.hpp>
#include <menps/mecom/common_policy.hpp>
#include <menps/mecom/rpc/rpc_policy.hpp>

namespace menps {
namespace medsm {

struct protector_space_proxy_policy
    : mecom::common_policy
    , mecom::rpc::rpc_policy
    , dsm_base_policy
{
    typedef protector_space::proxy      derived_type;
    typedef space_coherence_activater   interface_type;
    
    typedef protector_space             space_type;
    
    struct create_conf_type {
        mefdn::size_t  num_pages;
        mefdn::size_t  page_size;
        mefdn::size_t  block_size;
        void*           app_ptr;
        void*           sys_ptr;
        mefdn::size_t  index_in_file;
        bool            copy_data;
    };
    
    static const mecom::rpc::handler_id_t create_segment_handler_id = 701;
};

class protector_space::proxy
    : public basic_protector_space_proxy<protector_space_proxy_policy>
{
    typedef basic_protector_space_proxy<protector_space_proxy_policy> base;
    
public:
    explicit proxy(protector_space& sp)
        : a2a_(&sp)
    { }
    
    proxy(const proxy&) = delete;
    proxy& operator = (const proxy&) = delete;
    
    proxy(proxy&&) noexcept = default;
    proxy& operator = (proxy&&) noexcept = default;
    
private:
    friend class basic_protector_space_proxy<protector_space_proxy_policy>;
    
    protector_space* get_protector_space_at_proc(const mecom::process_id_t proc) const noexcept
    {
        return this->a2a_.at_process(proc);
    }
    
    mecom::structure::alltoall_ptr_group<protector_space> a2a_;
};

protector_space::proxy protector_space::make_proxy_collective()
{
    return protector_space::proxy(*this);
}

} // namespace medsm
} // namespace menps


