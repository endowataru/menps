
#include "alltoall_queue_pairs.hpp"
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/utility/move.hpp>
#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

void alltoall_queue_pairs::create(
    mgcom::endpoint& ep
,   collective::requester& coll
,   ibv_cq& cq
,   ibv_pd& pd
,   const device_attr_t& dev_attr
,   const mgdev::ibv::port_num_t port_num
) {
    ep_ = &ep;
    coll_ = &coll;
    
    const mgbase::size_t total_qp_count = ep_->number_of_processes() * qp_count_;
    
    qps_ = new queue_pair[total_qp_count];
    
    for (mgbase::size_t qp_id = 0; qp_id < total_qp_count; ++qp_id) {
        auto init_attr = mgdev::ibv::make_default_rc_qp_init_attr(dev_attr);
        init_attr.send_cq = &cq;
        init_attr.recv_cq = &cq;
        
        auto qp = mgdev::ibv::make_queue_pair(&pd, &init_attr);
        qp.init(port_num);
        
        qps_[qp_id] = mgbase::move(qp);
    }

    MGBASE_LOG_DEBUG("msg:Created all IBV queue_pairs.");
}

void alltoall_queue_pairs::destroy()
{
    qps_.reset();
    
    MGBASE_LOG_DEBUG("msg:Destroyed all IBV queue_pairs.");
}

void alltoall_queue_pairs::collective_start(
    const device_attr_t& device_attr
,   const port_attr_t& port_attr
,   const mgdev::ibv::port_num_t port_num
) {
    const mgbase::size_t total_qp_count = ep_->number_of_processes() * qp_count_;
    
    mgbase::scoped_ptr<mgbase::uint32_t []> local_qp_nums(
        new mgbase::uint32_t[total_qp_count]
    );
    mgbase::scoped_ptr<mgbase::uint32_t []> remote_qp_nums(
        new mgbase::uint32_t[total_qp_count]
    );
    mgbase::scoped_ptr<mgbase::uint16_t []> lids(
        new mgbase::uint16_t[total_qp_count]
    );
    
    // Extract qp_num.
    for (mgbase::size_t qp_id = 0; qp_id < total_qp_count; ++qp_id)
        local_qp_nums[qp_id] = qps_[qp_id].get_qp_num();
    
    // Exchange qp_num with all of the processes.
    mgcom::collective::alltoall(*coll_, &local_qp_nums[0], &remote_qp_nums[0], qp_count_);
    
    // Exchange lid with all of the processes.
    mgcom::collective::allgather(*coll_, &port_attr.lid, &lids[0], 1);
    
    // Start all of the queue_pairs.
    for (process_id_t proc = 0; proc < ep_->number_of_processes(); ++proc) {
        for (mgbase::size_t qp_index = 0; qp_index < qp_count_; ++qp_index) {
            const mgbase::size_t qp_id = proc * qp_count_ + qp_index;
            
            auto& qp = qps_[qp_id];
            
            mgdev::ibv::global_qp_id dest_id;
            dest_id.node_id = mgdev::ibv::make_node_id_from_lid(lids[proc]);
            dest_id.port_num = port_num;
            dest_id.qp_num = remote_qp_nums[qp_id];
            
            auto attr = mgdev::ibv::make_default_qp_attr(device_attr);
            qp.connect_to(dest_id, &attr);
        }
    }
    
    MGBASE_LOG_DEBUG("msg:Started all IBV queue_pairs.");
}

} // namespace ibv
} // namespace mgcom

