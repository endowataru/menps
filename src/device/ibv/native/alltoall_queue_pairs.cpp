
#include "alltoall_queue_pairs.hpp"
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

void alltoall_queue_pairs::create(mgcom::endpoint& ep, collective::requester& coll, ibv_context& ctx, ibv_cq& cq, ibv_pd& pd)
{
    ep_ = &ep;
    coll_ = &coll;
    
    const mgbase::size_t total_qp_count = ep_->number_of_processes() * qp_count_;
    
    qps_ = new queue_pair[total_qp_count];
    
    for (mgbase::size_t qp_id = 0; qp_id < total_qp_count; ++qp_id)
        qps_[qp_id].create(ctx, cq, pd);

    MGBASE_LOG_DEBUG("msg:Created all IBV queue_pairs.");
}

void alltoall_queue_pairs::destroy()
{
    qps_.reset();
    
    MGBASE_LOG_DEBUG("msg:Destroyed all IBV queue_pairs.");
}

void alltoall_queue_pairs::collective_start(const ibv_device_attr& device_attr, const ibv_port_attr& port_attr)
{
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
            qps_[qp_id].start(remote_qp_nums[qp_id], lids[proc], device_attr);
        }
    }
    
    MGBASE_LOG_DEBUG("msg:Started all IBV queue_pairs.");
}

} // namespace ibv
} // namespace mgcom

