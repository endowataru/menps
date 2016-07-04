
#pragma once

#include "queue_pair.hpp"
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

class alltoall_queue_pairs
{
public:
    alltoall_queue_pairs() MGBASE_EMPTY_DEFINITION
    
    void create(ibv_context& ctx, ibv_cq& cq, ibv_pd& pd)
    {
        conns_ = new queue_pair[mgcom::number_of_processes()];
        
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].create(ctx, cq, pd);

        MGBASE_LOG_DEBUG("msg:Created all IBV queue_pairs.");
    }
    
    void destroy()
    {
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].destroy();
        
        conns_.reset();
        
        MGBASE_LOG_DEBUG("msg:Destroyed all IBV queue_pairs.");
    }
    
    void collective_start(const ibv_device_attr& device_attr, const ibv_port_attr& port_attr)
    {
        mgbase::scoped_ptr<mgbase::uint32_t []> local_qp_nums(
            new mgbase::uint32_t[mgcom::number_of_processes()]
        );
        mgbase::scoped_ptr<mgbase::uint32_t []> remote_qp_nums(
            new mgbase::uint32_t[mgcom::number_of_processes()]
        );
        mgbase::scoped_ptr<mgbase::uint16_t []> lids(
            new mgbase::uint16_t[mgcom::number_of_processes()]
        );
        
        // Extract qp_num.
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            local_qp_nums[proc] = conns_[proc].get_qp_num();
        
        // Exchange qp_num with all of the processes.
        mgcom::collective::alltoall(&local_qp_nums[0], &remote_qp_nums[0], 1);
        
        // Exchange lid with all of the processes.
        mgcom::collective::allgather(&port_attr.lid, &lids[0], 1);
        
        // Start all of the queue_pairs.
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].start(remote_qp_nums[proc], lids[proc], device_attr);
        
        MGBASE_LOG_DEBUG("msg:Started all IBV queue_pairs.");
    }
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_post_send(
        const process_id_t  proc
    ,   ibv_send_wr&        wr
    ) {
        return conns_[proc].try_post_send(wr);
    }

private:
    mgbase::scoped_ptr<queue_pair []> conns_;
};

} // namespace ibv
} // namespace mgcom

