
#pragma once

#include "command_queue.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class endpoint;
class completion_selector;

class command_consumer
    : protected virtual command_queue
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
    
protected:
    explicit command_consumer(const config&);
    
public:
    virtual ~command_consumer();
    
    command_consumer(const command_consumer&) = delete;
    command_consumer& operator = (const command_consumer&) = delete;
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

