
#pragma once

#include <mgcom/common.hpp>
#include <mgdev/ibv/queue_pair.hpp>
#include <mgdev/ibv/attributes.hpp>
#include <mgdev/ibv/completion_queue.hpp>
#include "device/ibv/verbs.hpp"
#include "device/ibv/native/completer_set.hpp"
#include "device/ibv/command/tag_queue.hpp"

namespace mgcom {
namespace ibv {

class alltoall_queue_pairs
{
public:
    explicit alltoall_queue_pairs(const index_t qp_count)
        : qp_count_(qp_count)
    { }
    
    struct start_config {
        mgcom::endpoint&        ep;
        collective::requester&  coll;
        ibv_context&            dev_ctx;
        ibv_pd&                 pd;
        const device_attr_t&    dev_attr;
        const port_attr_t&      port_attr;
        port_num_t              port_num;
    };
    
    void collective_start(const start_config&);
    
    void destroy();
    
    queue_pair& get_qp(const process_id_t proc, const index_t qp_index)
    {
        MGBASE_ASSERT(qp_index < qp_count_);
        const auto qp_id = proc * qp_count_ + qp_index;
        return qps_[qp_id];
    }
    
    tag_queue& get_tag_queue(const process_id_t proc, const index_t qp_index)
    {
        MGBASE_ASSERT(qp_index < qp_count_);
        const auto qp_id = proc * qp_count_ + qp_index;
        return tag_queues_[qp_id];
    }
    
    completion_selector& get_comp_sel(const process_id_t proc, const index_t qp_index)
    {
        #ifdef MGCOM_IBV_SEPARATE_CQ
        MGBASE_ASSERT(qp_index < qp_count_);
        const auto qp_id = proc * qp_count_ + qp_index;
        return comp_set_->get_comp_sel(qp_id);
        #else
        return comp_set_->get_comp_sel(0);
        #endif
    }
    
private:
    const index_t                       qp_count_;
    
    mgbase::unique_ptr<completer_set>   comp_set_;
    mgbase::unique_ptr<queue_pair []>   qps_;
    mgbase::unique_ptr<tag_queue []>    tag_queues_;
};

} // namespace ibv
} // namespace mgcom
