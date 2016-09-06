
#pragma once

#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

class completion_queue;
class completion_selector;

class poll_thread
{
public:
    poll_thread(completion_queue&, completion_selector&);
    ~poll_thread();
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace ibv
} // namespace mgcom

