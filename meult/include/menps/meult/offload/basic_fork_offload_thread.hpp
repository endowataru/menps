
#pragma once

#include "basic_offload_thread.hpp"

namespace menps {
namespace meult {

template <typename Policy>
class basic_fork_offload_thread
    : public basic_offload_thread<Policy>
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
protected:
    basic_fork_offload_thread() = default;
    
    void start()
    {
        auto& self = this->derived();
        self.set_entrypoint(&basic_fork_offload_thread::loop, &self);
        
        while (!self.try_sleep()) {
            Policy::this_thread::yield();
        }
    }
    
    void stop()
    {
        auto& self = this->derived();
        
        self.finished_.store(true);
        
        self.force_notify();
    }
    
private:
    static void* loop(void* const self_ptr)
    {
        auto& self = *static_cast<derived_type*>(self_ptr);
        
        #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mefdn::get_cpu_clock();
        #endif
        
        while (MEFDN_LIKELY(
            ! self.finished_.load(mefdn::memory_order_relaxed)
        )) {
            if (MEFDN_UNLIKELY(
                ! self.dequeue_some()
            )) {
                #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
                const auto cur_clock = mefdn::get_cpu_clock();
                if (cur_clock - old_clock < MEULT_EXECUTOR_LIFETIME_CYCLES) {
                    Policy::this_thread::yield();
                }
                else {
                #endif
                    if (MEFDN_UNLIKELY(
                        ! self.has_remaining()
                    )) {
                        if (MEFDN_UNLIKELY(
                            self.try_sleep()
                        )) {
                            Policy::this_thread::detach();
                            return nullptr;
                        }
                    }
                #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
                }
                #endif
            }
            #if MEULT_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mefdn::get_cpu_clock();
            }
            #endif
            
            self.post_all();
        }
    }
};

} // namespace meult
} // namespace menps

