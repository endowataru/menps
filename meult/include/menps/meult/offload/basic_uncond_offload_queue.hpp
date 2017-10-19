
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/nonblocking/mpsc_locked_bounded_queue.hpp>

namespace menps {
namespace meult {

template <typename Policy>
class basic_uncond_offload_queue
    : public mefdn::static_mpsc_locked_bounded_queue<
        typename Policy::command_type
    ,   Policy::queue_size
    >
{
    typedef mefdn::static_mpsc_locked_bounded_queue<
        typename Policy::command_type
    ,   Policy::queue_size
    >
    base;
    
    typedef typename Policy::uncond_variable    uncond_type;
    
public:
    void notify_if_sleeping(typename base::enqueue_transaction& t)
    {
        if (t.is_sleeping()) { // (old_tail & 1) == 1
            MEFDN_LOG_DEBUG(
                "msg:Awake command consumer."
            );
            
            this->force_notify();
        }
    }
    
    void force_notify()
    {
        u_.notify();
    }
    
    bool try_sleep()
    {
        // TODO: rename "try_sleep" of base
        if (base::try_sleep()) {
            u_.wait();
            return true;
        }
        else
            return false;
    }
    
private:
    uncond_type u_;
};

} // namespace meult
} // namespace menps

