
#pragma once

#include <menps/mecom/rma/command_queue.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include "device/ibv/native/alltoall_queue_pairs.hpp"

namespace menps {
namespace mecom {
namespace ibv {

class rma_executor
{
public:
    struct config
    {
        alltoall_queue_pairs&   qps;
        mefdn::size_t          qp_index;
        
        rma::allocator&         alloc;
        mefdn::size_t          proc_first;
        mefdn::size_t          num_procs;
        bool                    reply_be;
    };
    
    explicit rma_executor(const config&);
    
    ~rma_executor();
    
    rma::command_queue& get_command_queue();
    
//private:
    class impl; // XXX
    
private:
    mefdn::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

