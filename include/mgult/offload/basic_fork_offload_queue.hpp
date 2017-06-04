
#pragma once

#include <mgult/common.hpp>
#include <mgbase/nonblocking/mpsc_locked_bounded_queue.hpp>

namespace mgult {

template <typename Policy>
class basic_fork_offload_queue
    : public mgbase::static_mpsc_locked_bounded_queue<
        typename Policy::command_type
    ,   Policy::queue_size
    >
{
    typedef mgbase::static_mpsc_locked_bounded_queue<
        typename Policy::command_type
    ,   Policy::queue_size
    >
    base;
    
public:
    void set_entrypoint(void* (*func)(void*), void* arg)
    {
        func_ = func;
        arg_ = arg;
    }
    
    void notify_if_sleeping(typename base::enqueue_transaction& t)
    {
        if (t.is_sleeping()) { // (old_tail & 1) == 1
            MGBASE_LOG_DEBUG(
                "msg:Create command consumer."
            );
            
            Policy::fork_fast(func_, arg_);
            
            /*#if 0
            ult::fork_parent_first_detached(func_, arg_);
            #else
            auto th = ult::thread(this->func_, this->arg_);
            th.detach();
            #endif*/
        }
    }
    
    void force_notify() {
        // do nothing
    }
    
private:
    void* (*func_)(void*);
    void* arg_;
    //ult::thread th_;
};

} // namespace mgult

