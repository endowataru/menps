
#pragma once

#include "protector_space.hpp"
#include "basic_protector_space_proxy.hpp"
#include <mgcom/structure/alltoall_ptr_group.hpp>
#include <mgcom/common_policy.hpp>
#include <mgcom/rpc/rpc_policy.hpp>

namespace mgdsm {

struct protector_space_proxy_policy
    : mgcom::common_policy
    , mgcom::rpc::rpc_policy
    , dsm_base_policy
{
    typedef protector_space::proxy      derived_type;
    typedef space_coherence_activater   interface_type;
    
    typedef protector_space             space_type;
    
    struct create_conf_type {
        mgbase::size_t  num_pages;
        mgbase::size_t  page_size;
        mgbase::size_t  block_size;
        void*           app_ptr;
        void*           sys_ptr;
        mgbase::size_t  index_in_file;
        bool            copy_data;
    };
    
    static const mgcom::rpc::handler_id_t create_segment_handler_id = 701;
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
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_1(proxy, base, a2a_)
    
private:
    friend class basic_protector_space_proxy<protector_space_proxy_policy>;
    
    protector_space* get_protector_space_at_proc(const mgcom::process_id_t proc) const MGBASE_NOEXCEPT
    {
        return this->a2a_.at_process(proc);
    }
    
    mgcom::structure::alltoall_ptr_group<protector_space> a2a_;
};

protector_space::proxy protector_space::make_proxy_collective()
{
    return protector_space::proxy(*this);
}

} // namespace mgdsm


