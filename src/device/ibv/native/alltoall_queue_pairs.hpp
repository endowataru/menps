
#pragma once

#include <mgcom/common.hpp>
#include <mgdev/ibv/queue_pair.hpp>
#include <mgdev/ibv/attributes.hpp>
#include <mgbase/scoped_ptr.hpp>
#ifdef MGCOM_IBV_SEPARATE_CQ
#include <mgdev/ibv/completion_queue.hpp>
#include <mgdev/ibv/device_context.hpp>
#endif

namespace mgcom {
namespace ibv {

using mgdev::ibv::queue_pair;
using mgdev::ibv::device_attr_t;
using mgdev::ibv::port_attr_t;
using mgdev::ibv::port_num_t;

#ifdef MGCOM_IBV_SEPARATE_CQ
using mgdev::ibv::completion_queue;
using mgdev::ibv::device_context;
#endif

class alltoall_queue_pairs
{
public:
    explicit alltoall_queue_pairs(const index_t qp_count)
        : qp_count_(qp_count) { }
    
    struct start_config {
        mgcom::endpoint&        ep;
        collective::requester&  coll;
        #ifdef MGCOM_IBV_SEPARATE_CQ
        device_context&         dev_ctx;
        #else
        ibv_cq&                 cq;
        #endif
        ibv_pd&                 pd;
        const device_attr_t&    dev_attr;
        const port_attr_t&      port_attr;
        port_num_t              port_num;
    };
    
    void collective_start(const start_config&);
    
    void destroy();
    
    mgbase::uint32_t get_qp_num_of_proc(const process_id_t proc, const index_t qp_index)
    {
        return get_qp(proc, qp_index).get_qp_num();
    }
    
    index_t get_qp_count() const MGBASE_NOEXCEPT { return qp_count_; }
    
    #ifdef MGCOM_IBV_SEPARATE_CQ
    completion_queue& get_cq(const process_id_t proc, const index_t qp_index) const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(qp_index < qp_count_);
        const auto id = proc * qp_count_ + qp_index;
        return cqs_[id];
    }
    #endif
    
    queue_pair& get_qp(const process_id_t proc, const index_t qp_index)
    {
        MGBASE_ASSERT(qp_index < qp_count_);
        const auto qp_id = proc * qp_count_ + qp_index;
        return qps_[qp_id];
    }
    
private:
    const index_t                       qp_count_;
    
    #ifdef MGCOM_IBV_SEPARATE_CQ
    mgbase::unique_ptr<completion_queue []> cqs_;
    #endif
    
    mgbase::unique_ptr<queue_pair []>   qps_;
};

} // namespace ibv
} // namespace mgcom

