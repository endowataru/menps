
#pragma once

#include "command_queue.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
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
        mgbase::size_t          qp_index;
        
        rma::allocator&         alloc;
        mgbase::size_t          proc_first;
        mgbase::size_t          num_procs;
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
    mgbase::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mgcom
