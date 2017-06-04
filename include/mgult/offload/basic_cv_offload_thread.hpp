
#pragma once

#include "basic_offload_thread.hpp"

namespace mgult {

template <typename Policy>
class basic_cv_offload_thread
    : public basic_offload_thread<Policy>
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::thread_type    thread_type;
    
protected:
    basic_cv_offload_thread() = default;
    
    void start()
    {
        th_ = thread_type(starter{*this});
    }
    
    void stop()
    {
        auto& self = this->derived();
        
        self.finished_.store(true);
        
        self.force_notify();
        
        th_.join();
    }
    
private:
    struct starter
    {
       basic_cv_offload_thread& self;
       
       void operator() () {
            self.loop();
       }
    };
    
    void loop()
    {
        auto& self = this->derived();
        
        #if MGULT_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mgbase::get_cpu_clock();
        #endif
        
        while (MGBASE_LIKELY(
            ! self.finished_.load(mgbase::memory_order_relaxed)
        )) {
            if (MGBASE_UNLIKELY(
                ! self.dequeue_some()
            )) {
                if (MGBASE_UNLIKELY(
                    ! self.has_remaining()
                )) {
                    #if MGULT_EXECUTOR_LIFETIME_CYCLES > 0
                    const auto cur_clock = mgbase::get_cpu_clock();
                    if (cur_clock - old_clock < MGULT_EXECUTOR_LIFETIME_CYCLES) {
                        Policy::this_thread::yield();
                    }
                    else {
                    #endif
                        if (MGBASE_UNLIKELY(
                            ! self.try_sleep()
                        )) {
                            Policy::this_thread::yield();
                        }
                    #if MGULT_EXECUTOR_LIFETIME_CYCLES > 0
                    }
                    #endif
                } else {
                    Policy::this_thread::yield();
                }
            }
            #if MGULT_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mgbase::get_cpu_clock();
            }
            #endif
            
            self.post_all();
        }
    }
    
    thread_type th_;
};

} // namespace mgult

