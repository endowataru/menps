
#pragma once

#include <mgcom/rma/command_queue.hpp>
#include <mgbase/unique_ptr.hpp>
#include "device/ibv/native/alltoall_queue_pairs.hpp"

namespace mgcom {
namespace ibv {

class rma_executor
{
public:
    struct config
    {
        alltoall_queue_pairs&   qps;
        mgbase::size_t          qp_index;
        
        rma::allocator&         alloc;
        mgbase::size_t          proc_first;
        mgbase::size_t          num_procs;
        bool                    reply_be;
    };
    
    explicit rma_executor(const config&);
    
    ~rma_executor();
    
    rma::command_queue& get_command_queue();
    
//private:
    class impl; // XXX
    
private:
    mgbase::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mgcom

