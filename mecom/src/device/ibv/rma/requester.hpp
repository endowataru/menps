
#pragma once

#include <menps/mecom/rma/requester.hpp>
#include <menps/mecom/rma/allocator.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class alltoall_queue_pairs;
class completion_selector;

struct requester_config
{
    alltoall_queue_pairs&   qps;
    rma::allocator&         alloc;
    mecom::endpoint&        ep;
    bool                    reply_be;
    mefdn::size_t          num_qps_per_proc;
};

mefdn::unique_ptr<rma::requester> make_rma_direct_requester(const requester_config&);

mefdn::unique_ptr<rma::requester> make_scheduled_rma_requester(const requester_config&);

mefdn::unique_ptr<rma::requester> make_rma_offload_requester(const requester_config&);

} // namespace ibv
} // namespace mecom
} // namespace menps

