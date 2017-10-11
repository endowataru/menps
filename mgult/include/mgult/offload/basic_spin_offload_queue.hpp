
#pragma once

#include <mgult/common.hpp>
#include <mgbase/nonblocking/mpsc_bounded_queue.hpp>

namespace mgult {

template <typename Policy>
class basic_spin_offload_queue
    : public mgbase::static_mpsc_bounded_queue<
        typename Policy::command_type
    ,   Policy::queue_size
    >
{
    typedef mgbase::static_mpsc_bounded_queue<
        typename Policy::command_type
    ,   Policy::queue_size
    >
    base;
    
public:
    void notify_if_sleeping(typename base::enqueue_transaction& /*unused*/)
    {
        // do nothing
    }
    
    void force_notify()
    {
        // do nothing
    }
    
    bool try_sleep()
    {
        return false;
    }
};

} // namespace mgult

