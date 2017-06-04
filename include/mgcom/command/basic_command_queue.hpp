
#pragma once

#include <mgcom/common.hpp>
#include <mgult/offload/basic_spin_offload_queue.hpp>
#if 0
#include <mgcom/common.hpp>
#include <mgbase/nonblocking/mpsc_bounded_queue.hpp>
#endif

namespace mgcom {

template <typename Policy>
class basic_command_queue
    : public mgult::basic_spin_offload_queue<Policy>
{ };

#if 0
template <typename Policy>
class basic_command_queue
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
#endif

} // namespace mgcom
