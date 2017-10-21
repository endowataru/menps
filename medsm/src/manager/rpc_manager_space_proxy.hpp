
#pragma once

#include "rpc_manager_space.hpp"
#include <menps/mecom/structure/alltoall_ptr_group.hpp>
#include <menps/mecom/rpc/rpc_policy.hpp>

namespace menps {
namespace medsm {

#if 0
struct rpc_manager_space_proxy_policy
    : mecom::rpc::rpc_policy
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
    
    proxy(proxy&&) noexcept = default;
    proxy& operator = (proxy&&) noexcept = default;
    
    rpc_manager_space& get_space() const noexcept {
        return *sp_;
    }
    
    // TODO: make these members private
    
    mecom::process_id_t get_manager_proc(const page_id_t pg_id) const noexcept
    {
        return static_cast<mecom::process_id_t>(pg_id % mecom::number_of_processes());
    }
    rpc_manager_space* get_manager_space_at_proc(const mecom::process_id_t proc) const noexcept
    {
        return this->a2a_.at_process(proc);
    }
    
    virtual void make_segment(const segment_id_t seg_id, const segment_conf& conf) MEFDN_OVERRIDE {
        sp_->make_segment(seg_id, conf);
    }
    
    virtual manager_segment_proxy_ptr make_segment_proxy(segment_id_t) MEFDN_OVERRIDE;
    
private:
    rpc_manager_space* sp_;
    
    mecom::structure::alltoall_ptr_group<rpc_manager_space> a2a_;
};

rpc_manager_space::proxy rpc_manager_space::make_proxy_collective()
{
    return rpc_manager_space::proxy(*this);
}

} // namespace medsm
} // namespace menps

