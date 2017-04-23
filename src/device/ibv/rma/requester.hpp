
#pragma once

#include <mgcom/rma/requester.hpp>
#include <mgcom/rma/allocator.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class alltoall_queue_pairs;
class completion_selector;

struct requester_config
{
    alltoall_queue_pairs&   qps;
    #ifndef MGCOM_IBV_SEPARATE_CQ
    completion_selector&    comp_sel;
    #endif
    rma::allocator&         alloc;
    mgcom::endpoint&        ep;
    bool                    reply_be;
    mgbase::size_t          num_qps_per_proc;
};

mgbase::unique_ptr<rma::requester> make_rma_direct_requester(const requester_config&);

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(const requester_config&);

} // namespace ibv
} // namespace mgcom

