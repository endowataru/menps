
#include "alltoall_queue_pairs.hpp"
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/utility/move.hpp>
#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

using mgdev::ibv::qp_num_t;
using mgdev::ibv::lid_t;

void alltoall_queue_pairs::collective_start(const start_config& conf)
{
    auto& ep = conf.ep;
    
    const mgbase::size_t total_qp_count = ep.number_of_processes() * qp_count_;
    
    #ifdef MGCOM_IBV_SEPARATE_CQ
    cqs_ = mgbase::make_unique<completion_queue []>(total_qp_count);
    #endif
    
    qps_ = mgbase::make_unique<queue_pair []>(total_qp_count);
    
    for (mgbase::size_t qp_id = 0; qp_id < total_qp_count; ++qp_id) {
        #ifdef MGCOM_IBV_SEPARATE_CQ
        cqs_[qp_id] = mgdev::ibv::make_completion_queue(conf.dev_ctx.get());
        #endif
        
        auto init_attr = mgdev::ibv::make_default_rc_qp_init_attr(conf.dev_attr);
        #ifdef MGCOM_IBV_SEPARATE_CQ
        init_attr.send_cq = cqs_[qp_id].get();
        init_attr.recv_cq = cqs_[qp_id].get();
        #else
        init_attr.send_cq = &conf.cq;
        init_attr.recv_cq = &conf.cq;
        #endif
        
        auto qp = mgdev::ibv::make_queue_pair(&conf.pd, &init_attr);
        qp.init(conf.port_num);
        
        qps_[qp_id] = mgbase::move(qp);
    }

    MGBASE_LOG_DEBUG("msg:Created all IBV queue_pairs.");
    
    const auto local_qp_nums  = mgbase::make_unique<qp_num_t []>(total_qp_count);
    const auto remote_qp_nums = mgbase::make_unique<qp_num_t []>(total_qp_count);
    const auto lids           = mgbase::make_unique<lid_t []>(total_qp_count);
    
    // Extract qp_num.
    for (mgbase::size_t qp_id = 0; qp_id < total_qp_count; ++qp_id)
        local_qp_nums[qp_id] = qps_[qp_id].get_qp_num();
    
    // Exchange qp_num with all of the processes.
    mgcom::collective::alltoall(conf.coll, &local_qp_nums[0], &remote_qp_nums[0], qp_count_);
    
    // Exchange lid with all of the processes.
    mgcom::collective::allgather(conf.coll, &conf.port_attr.lid, &lids[0], 1);
    
    // Start all of the queue_pairs.
    for (process_id_t proc = 0; proc < ep.number_of_processes(); ++proc) {
        for (mgbase::size_t qp_index = 0; qp_index < qp_count_; ++qp_index) {
            const mgbase::size_t qp_id = proc * qp_count_ + qp_index;
            
            auto& qp = qps_[qp_id];
            
            mgdev::ibv::global_qp_id dest_id;
            dest_id.node_id = mgdev::ibv::make_node_id_from_lid(lids[proc]);
            dest_id.port_num = conf.port_num;
            dest_id.qp_num = remote_qp_nums[qp_id];
            
            auto attr = mgdev::ibv::make_default_qp_attr(conf.dev_attr);
            qp.connect_to(dest_id, &attr);
        }
    }
    
    MGBASE_LOG_DEBUG("msg:Started all IBV queue_pairs.");
}

void alltoall_queue_pairs::destroy()
{
    qps_.reset();
    
    #ifdef MGCOM_IBV_SEPARATE_CQ
    cqs_.reset();
    #endif
    
    MGBASE_LOG_DEBUG("msg:Destroyed all IBV queue_pairs.");
}

} // namespace ibv
} // namespace mgcom

