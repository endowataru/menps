
#pragma once

#include "basic_rpc_manager_space_proxy.hpp"
#include "rpc_manager_space.hpp"
#include "manager_space_proxy.hpp"
#include <mgcom/structure/alltoall_ptr_group.hpp>
#include <mgcom/rpc/rpc_policy.hpp>

namespace mgdsm {

struct rpc_manager_space_proxy_policy
    : mgcom::rpc::rpc_policy
{
    typedef rpc_manager_space::proxy        derived_type;
    
    typedef rpc_manager_space               space_type;
    
    typedef segment_id_t                    segment_id_type;
    
    typedef mgcom::process_id_t             process_id_type;
    
    struct create_conf_type {
        mgbase::size_t  num_pages;
        mgbase::size_t  page_size;
        mgbase::size_t  block_size;
    };
    
    static const mgcom::rpc::handler_id_t create_segment_handler_id = 301;
    
    static mgbase::size_t number_of_processes() MGBASE_NOEXCEPT {
        return mgcom::number_of_processes();
    }
};

class rpc_manager_space::proxy
    : public basic_rpc_manager_space_proxy<rpc_manager_space_proxy_policy>
    , public manager_space_proxy
{
public:
    explicit proxy(rpc_manager_space& sp)
        : sp_(sp)
        , a2a_(&sp)
    { }
    
    virtual manager_segment_proxy_ptr make_segment_proxy(segment_id_t) MGBASE_OVERRIDE;
        // defined in rpc_manager_segment_proxy.hpp
    
    rpc_manager_space& get_space() const MGBASE_NOEXCEPT {
        return sp_;
    }
    
    // TODO: make these members private
    
    mgcom::process_id_t get_manager_proc(const page_id_t pg_id) const MGBASE_NOEXCEPT
    {
        return pg_id % mgcom::number_of_processes();
    }
    rpc_manager_space* get_manager_space_at_proc(const mgcom::process_id_t proc) const MGBASE_NOEXCEPT
    {
        return this->a2a_.at_process(proc);
    }
    
private:
    rpc_manager_space& sp_;
    
    mgcom::structure::alltoall_ptr_group<rpc_manager_space> a2a_;
};

rpc_manager_space::proxy rpc_manager_space::make_proxy_collective()
{
    return rpc_manager_space::proxy(*this);
}

} // namespace mgdsm

