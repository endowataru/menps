
#pragma once

#include "basic_rpc_manager_segment_proxy.hpp"
#include "rpc_manager_space.hpp"
#include "rpc_manager_segment.hpp"
#include "rpc_manager_page_accessor.hpp"
#include "rpc_manager_space_proxy.hpp"
#include <menps/mecom/rpc/rpc_policy.hpp>

namespace menps {
namespace medsm {

struct rpc_manager_segment_proxy_policy
    : mecom::rpc::rpc_policy
{
    typedef rpc_manager_segment::proxy      derived_type;
    
    typedef manager_segment_proxy           interface_type;
    
    typedef rpc_manager_space               space_type;
    typedef rpc_manager_page                page_type;
    
    typedef page_id_t                       page_id_type;
    typedef segment_id_t                    segment_id_type;
    
    typedef mecom::rma::paired_local_ptr<void>  plptr_type;
    
    typedef rpc_manager_page_invalidator    invalidator_type;
    
    static const mecom::rpc::handler_id_t acquire_read_handler_id = 401;
    static const mecom::rpc::handler_id_t release_read_handler_id = 402;
    static const mecom::rpc::handler_id_t acquire_write_handler_id = 403;
    static const mecom::rpc::handler_id_t release_write_handler_id = 404;
    static const mecom::rpc::handler_id_t assign_reader_handler_id = 405;
    static const mecom::rpc::handler_id_t assign_writer_handler_id = 406;
};

class rpc_manager_segment::proxy
    : public basic_rpc_manager_segment_proxy<rpc_manager_segment_proxy_policy>
{
    typedef rpc_manager_segment_proxy_policy    policy;
    
public:
    /*implicit*/ proxy(
        rpc_manager_space::proxy&   sp_pr
    ,   const segment_id_t          seg_id
    )
        : sp_pr_(sp_pr)
        , seg_id_(seg_id)
    { }
    
private:
    friend class basic_rpc_manager_segment_proxy<rpc_manager_segment_proxy_policy>;
    
    rpc_manager_space::proxy& get_space_proxy() const noexcept {
        return this->sp_pr_;
    }
    segment_id_t get_segment_id() const noexcept {
        return this->seg_id_;
    }
    
    rpc_manager_segment::accessor get_accessor() const noexcept
    {
        auto& sp = this->sp_pr_.get_space();
        return sp.get_segment_accessor(this->seg_id_);
    }
    
    rpc_manager_space::proxy&   sp_pr_;
    segment_id_t                seg_id_;
};

manager_segment_proxy_ptr rpc_manager_space::proxy::make_segment_proxy(const segment_id_t seg_id)
{
    return mefdn::make_unique<rpc_manager_segment::proxy>(*this, seg_id);
}

} // namespace medsm
} // namespace menps

