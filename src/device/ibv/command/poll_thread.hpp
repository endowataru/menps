
#pragma once

#include <mgdev/ibv/completion_queue.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class completion_selector;

class poll_thread
{
public:
    poll_thread(mgdev::ibv::completion_queue&, completion_selector&);
    ~poll_thread();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mgcom

