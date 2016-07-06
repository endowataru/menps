
#pragma once

#include <mgcom/common.hpp>
#include "queue_pair.hpp"
#include <mgbase/scoped_ptr.hpp>

namespace mgcom {
namespace ibv {

class alltoall_queue_pairs
{
public:
    alltoall_queue_pairs() MGBASE_EMPTY_DEFINITION
    
    void create(ibv_context& ctx, ibv_cq& cq, ibv_pd& pd);
    
    void destroy();
    
    void collective_start(const ibv_device_attr&, const ibv_port_attr&);
    
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

