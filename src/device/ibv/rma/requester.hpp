
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
    completion_selector&    comp_sel;
    rma::allocator&         alloc;
    mgcom::endpoint&        ep;
    bool                    reply_be;
};

mgbase::unique_ptr<rma::requester> make_rma_direct_requester(const requester_config&);

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(const requester_config&);

} // namespace ibv
} // namespace mgcom

