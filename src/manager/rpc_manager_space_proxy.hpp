
#pragma once

#include "rpc_manager_space.hpp"
#include <mgcom/structure/alltoall_ptr_group.hpp>
#include <mgcom/rpc/rpc_policy.hpp>

namespace mgdsm {

#if 0
struct rpc_manager_space_proxy_policy
    : mgcom::rpc::rpc_policy
{
    typedef rpc_manager_space::proxy        derived_type;
};
#endif

class rpc_manager_space::proxy
    //: public basic_rpc_manager_space_proxy<rpc_manager_space_proxy_policy>
    : public manager_space
{
    //typedef basic_rpc_manager_space_proxy<rpc_manager_space_proxy_policy>   base;
    
public:
    explicit proxy(rpc_manager_space& sp)
        : sp_(&sp)
        , a2a_(&sp)
    { }
    
    proxy(const proxy&) = delete;
    proxy& operator = (const proxy&) = delete;
    
    // Note: precisely, there are two base classes
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_2(proxy, sp_, a2a_)
    
    rpc_manager_space& get_space() const MGBASE_NOEXCEPT {
        return *sp_;
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
    
    virtual void make_segment(const segment_id_t seg_id, const segment_conf& conf) MGBASE_OVERRIDE {
        sp_->make_segment(seg_id, conf);
    }
    
    virtual manager_segment_proxy_ptr make_segment_proxy(segment_id_t) MGBASE_OVERRIDE;
    
private:
    rpc_manager_space* sp_;
    
    mgcom::structure::alltoall_ptr_group<rpc_manager_space> a2a_;
};

rpc_manager_space::proxy rpc_manager_space::make_proxy_collective()
{
    return rpc_manager_space::proxy(*this);
}

} // namespace mgdsm

