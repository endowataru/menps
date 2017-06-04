
#pragma once

#include "basic_offload_thread.hpp"

namespace mgult {

template <typename Policy>
class basic_fork_offload_thread
    : public basic_offload_thread<Policy>
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
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
        
        #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mgbase::get_cpu_clock();
        #endif
        
        while (MGBASE_LIKELY(
            ! self.finished_.load(mgbase::memory_order_relaxed)
        )) {
            if (MGBASE_UNLIKELY(
                ! self.dequeue_some()
            )) {
                #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
                const auto cur_clock = mgbase::get_cpu_clock();
                if (cur_clock - old_clock < MGCOM_EXECUTOR_LIFETIME_CYCLES) {
                    Policy::this_thread::yield();
                }
                else {
                #endif
                    if (MGBASE_UNLIKELY(
                        ! self.has_remaining()
                    )) {
                        if (MGBASE_UNLIKELY(
                            self.try_sleep()
                        )) {
                            Policy::this_thread::detach();
                            return MGBASE_NULLPTR;
                        }
                    }
                #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
                }
                #endif
            }
            #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mgbase::get_cpu_clock();
            }
            #endif
            
            self.post_all();
        }
    }
};

} // namespace mgult

