
#pragma once

#include "basic_rpc_manager_segment_proxy.hpp"
#include "rpc_manager_space_proxy.hpp"
#include "rpc_manager_segment.hpp"
#include "rpc_manager_page_accessor.hpp"
#include <mgcom/rpc/rpc_policy.hpp>

namespace mgdsm {

struct rpc_manager_segment_proxy_policy
    : mgcom::rpc::rpc_policy
{
    typedef rpc_manager_segment::proxy      derived_type;
    
    typedef manager_segment_proxy           interface_type;
    
    typedef rpc_manager_space               space_type;
    typedef rpc_manager_page                page_type;
    
    typedef page_id_t                       page_id_type;
    typedef segment_id_t                    segment_id_type;
    
    typedef mgcom::rma::paired_local_ptr<void>  plptr_type;
    
    static const mgcom::rpc::handler_id_t acquire_read_handler_id = 401;
    static const mgcom::rpc::handler_id_t release_read_handler_id = 402;
    static const mgcom::rpc::handler_id_t acquire_write_handler_id = 403;
    static const mgcom::rpc::handler_id_t release_write_handler_id = 404;
    static const mgcom::rpc::handler_id_t assign_reader_handler_id = 405;
    static const mgcom::rpc::handler_id_t assign_writer_handler_id = 406;
};

class rpc_manager_segment::proxy
    : public basic_rpc_manager_segment_proxy<rpc_manager_segment_proxy_policy>
{
    typedef rpc_manager_segment_proxy_policy    policy;
    
public:
    proxy(
        rpc_manager_space::proxy&   sp_pr
    ,   const segment_id_t          seg_id
    )
        : sp_pr_(sp_pr)
        , seg_id_(seg_id)
    { }
    
private:
    friend class basic_rpc_manager_segment_proxy<rpc_manager_segment_proxy_policy>;
    
    rpc_manager_space::proxy& get_space_proxy() const MGBASE_NOEXCEPT {
        return this->sp_pr_;
    }
    segment_id_t get_segment_id() const MGBASE_NOEXCEPT {
        return this->seg_id_;
    }
    
    rpc_manager_segment::accessor get_accessor() const MGBASE_NOEXCEPT
    {
        auto& sp = this->sp_pr_.get_space();
        
        return sp.get_segment_accessor(this->seg_id_);
    }
    
    rpc_manager_space::proxy&   sp_pr_;
    segment_id_t                seg_id_;
};

manager_segment_proxy_ptr rpc_manager_space::proxy::make_segment_proxy(const segment_id_t seg_id)
{
    return mgbase::make_unique<rpc_manager_segment::proxy>(*this, seg_id);
}

} // namespace mgdsm

