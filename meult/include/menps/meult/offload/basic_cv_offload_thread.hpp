
#pragma once

#include "basic_offload_thread.hpp"

namespace menps {
namespace meult {

template <typename Policy>
class basic_cv_offload_thread
    : public basic_offload_thread<Policy>
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::thread     thread_type;
    
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
        
        #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mefdn::get_cpu_clock();
        #endif
        
        while (MEFDN_LIKELY(
            ! self.finished_.load(mefdn::memory_order_relaxed)
        )) {
            if (MEFDN_UNLIKELY(
                ! self.dequeue_some()
            )) {
                if (MEFDN_UNLIKELY(
                    ! self.has_remaining()
                )) {
                    #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
                    const auto cur_clock = mefdn::get_cpu_clock();
                    if (cur_clock - old_clock < MEULT_EXECUTOR_LIFETIME_CYCLES) {
                        Policy::this_thread::yield();
                    }
                    else {
                    #endif
                        if (MEFDN_UNLIKELY(
                            ! self.try_sleep()
                        )) {
                            Policy::this_thread::yield();
                        }
                    #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
                    }
                    #endif
                } else {
                    Policy::this_thread::yield();
                }
            }
            #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mefdn::get_cpu_clock();
            }
            #endif
            
            self.post_all();
        }
    }
    
protected: // XXX: for meuct's workaround
    thread_type th_;
};

} // namespace meult
} // namespace menps

