
#pragma once

#include <mgult/common.hpp>
#include <mgbase/nonblocking/mpsc_locked_bounded_queue.hpp>

namespace mgult {

template <typename Policy>
class basic_cv_offload_queue
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
    
    typedef typename Policy::mutex              mutex_type;
    typedef typename Policy::condition_variable cv_type;
    typedef typename Policy::unique_mutex_lock  unique_lock_type;
    
public:
    void notify_if_sleeping(typename base::enqueue_transaction& t)
    {
        if (t.is_sleeping()) { // (old_tail & 1) == 1
            MGBASE_LOG_DEBUG(
                "msg:Awake command consumer."
            );
            
            this->force_notify();
        }
    }
    
    void force_notify()
    {
        const auto lk = this->get_lock();
        cv_.notify_one();
    }
    
    bool try_sleep()
    {
        auto lk = this->get_lock();
        
        // TODO: rename "try_sleep" of base
        if (base::try_sleep()) {
            cv_.wait(lk);
            return true;
        }
        else
            return false;
    }
    
private:
    unique_lock_type get_lock() {
        return unique_lock_type(mtx_);
    }
    
    mutex_type  mtx_;
    cv_type     cv_;
};

} // namespace mgult

