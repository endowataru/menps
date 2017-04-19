
#pragma once

#include <mgcom/common.hpp>
#include <mgdev/ibv/queue_pair.hpp>
#include <mgdev/ibv/attributes.hpp>
#include <mgbase/scoped_ptr.hpp>

namespace mgcom {
namespace ibv {

using mgdev::ibv::queue_pair;
using mgdev::ibv::device_attr_t;
using mgdev::ibv::port_attr_t;
using mgdev::ibv::port_num_t;

class alltoall_queue_pairs
{
public:
    explicit alltoall_queue_pairs(const index_t qp_count)
        : qp_count_(qp_count) { }
    
    struct start_config {
        mgcom::endpoint&        ep;
        collective::requester&  coll;
        ibv_cq&                 cq;
        ibv_pd&                 pd;
        const device_attr_t&    dev_attr;
        const port_attr_t&      port_attr;
        port_num_t              port_num;
    };
    
    void collective_start(const start_config&);
    
    void destroy();
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_post_send(
        const process_id_t  proc
    ,   const index_t       qp_index
    ,   ibv_send_wr&        wr
    ,   ibv_send_wr** const bad_wr
    ) {
        return get_qp(proc, qp_index).try_post_send(wr, bad_wr);
    }
    
    mgbase::uint32_t get_qp_num_of_proc(const process_id_t proc, const index_t qp_index)
    {
        return get_qp(proc, qp_index).get_qp_num();
    }
    
    index_t get_qp_count() const MGBASE_NOEXCEPT { return qp_count_; }
    
private:
    queue_pair& get_qp(const process_id_t proc, const index_t qp_index)
    {
        MGBASE_ASSERT(qp_index < qp_count_);
        const auto qp_id = proc * qp_count_ + qp_index;
        return qps_[qp_id];
    }
    
    const index_t                       qp_count_;
    mgbase::unique_ptr<queue_pair []>   qps_;
};

} // namespace ibv
} // namespace mgcom

