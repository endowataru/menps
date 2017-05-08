
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/nonblocking/mpsc_locked_bounded_queue.hpp>

namespace mgcom {

template <typename Policy>
class basic_locked_command_queue
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
    
    #ifdef MGCOM_FORK_OFFLOAD_THREAD
public:
    void set_entrypoint(void (*func)(void*), void* arg)
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
            
            auto th = ult::thread(this->func_, this->arg_);
            th.detach();
            
            //th_ = mgbase::move(th);
        }
    }
    
    void force_notify() {
        // do nothing
    }
    
    // TODO: breaking privacy
    /*ult::thread& get_thread() {
        return th_;
    }*/
    
private:
    void (*func_)(void*);
    void* arg_;
    //ult::thread th_;
    
    #else
    
    typedef ult::unique_lock<ult::mutex>    unique_lock_type;
    
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
    
    ult::mutex              mtx_;
    ult::condition_variable cv_;
    #endif
};

} // namespace mgcom

