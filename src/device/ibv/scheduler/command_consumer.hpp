
#pragma once

#include "command_queue.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class endpoint;

class command_consumer
    : protected virtual command_queue
{
public:
    struct config
    {
        endpoint&    ep;
        process_id_t proc_first;
        process_id_t num_procs;
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

