
#pragma once

#include <menps/medev/ibv/completion_queue.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class completion_selector;

class poll_thread
{
public:
    poll_thread(
        medev::ibv::completion_queue&
    ,   completion_selector&
    );
    
    ~poll_thread();
    
private:
    class impl;
    mefdn::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

