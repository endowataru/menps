
#pragma once

#include <menps/mecom/common.hpp>

#ifdef MECOM_FORK_EXECUTOR_THREAD
    #include <menps/meult/offload/basic_fork_offload_thread.hpp>
#else
    #include <menps/meult/offload/basic_cv_offload_thread.hpp>
#endif

#if 0
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/crtp_base.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#endif

namespace menps {
namespace mecom {

template <typename Policy>
class basic_offload_thread
#ifdef MECOM_FORK_EXECUTOR_THREAD
    : public meult::basic_fork_offload_thread<Policy>
#else
    : public meult::basic_cv_offload_thread<Policy>
#endif
{ };

#if 0

template <typename Policy>
class basic_offload_thread
{
    MEFDN_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::thread_type    thread_type;
    
protected:
    basic_offload_thread()
        : finished_{false}
    { }
    
    void start()
    {
        #ifdef MECOM_FORK_EXECUTOR_THREAD
        auto& self = this->derived();
        self.set_entrypoint(&basic_offload_thread::loop, &self);
        
        while (!self.try_sleep()) {
            ult::this_thread::yield();
        }
        #else
        th_ = thread_type(starter{*this});
        #endif
    }
    
    void stop()
    {
        auto& self = this->derived();
        
        finished_.store(true);
        
        self.force_notify();
        
        #ifndef MECOM_FORK_EXECUTOR_THREAD
        th_.join();
        #endif
    }
    
private:
    #ifdef MECOM_FORK_EXECUTOR_THREAD
    static void* loop(void* const self_ptr)
    {
        auto& self = *static_cast<derived_type*>(self_ptr);
        
        #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mefdn::get_cpu_clock();
        #endif
        
        while (MEFDN_LIKELY(
            ! self.finished_.load(mefdn::memory_order_relaxed)
        )) {
            if (MEFDN_UNLIKELY(
                ! self.dequeue_some()
            )) {
                #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
                const auto cur_clock = mefdn::get_cpu_clock();
                if (cur_clock - old_clock < MECOM_EXECUTOR_LIFETIME_CYCLES) {
                    ult::this_thread::yield();
                }
                else {
                #endif
                    if (MEFDN_UNLIKELY(
                        ! self.has_remaining()
                    )) {
                        if (MEFDN_UNLIKELY(
                            self.try_sleep()
                        )) {
                            ult::this_thread::detach();
                            return nullptr;
                        }
                    }
                #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
                }
                #endif
            }
            #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mefdn::get_cpu_clock();
            }
            #endif
            
            self.post_all();
        }
        
    }
    #else
    struct starter
    {
       basic_offload_thread& self;
       
       void operator() () {
            self.loop();
       }
    };
    
    void loop()
    {
        auto& self = this->derived();
        
        #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mefdn::get_cpu_clock();
        #endif
        
        while (MEFDN_LIKELY(
            ! finished_.load(mefdn::memory_order_relaxed)
        )) {
            if (MEFDN_UNLIKELY(
                ! this->dequeue_some()
            )) {
                if (MEFDN_UNLIKELY(
                    ! self.has_remaining()
                )) {
                    #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
                    const auto cur_clock = mefdn::get_cpu_clock();
                    if (cur_clock - old_clock < MECOM_EXECUTOR_LIFETIME_CYCLES) {
                        ult::this_thread::yield();
                    }
                    else {
                    #endif
                        if (MEFDN_UNLIKELY(
                            ! self.try_sleep()
                        )) {
                            ult::this_thread::yield();
                        }
                    #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
                    }
                    #endif
                } else {
                    ult::this_thread::yield();
                }
            }
            #if MECOM_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mefdn::get_cpu_clock();
            }
            #endif
            
            self.post_all();
        }
    }
    #endif
    
    bool dequeue_some()
    {
        auto& self = this->derived();
        
        auto t = self.try_dequeue();
        if (!t.valid()) {
            return false;
        }
        
        mefdn::size_t n = 0;
        
        // TODO: exception safety
        
        MEFDN_RANGE_BASED_FOR(auto&& cmd, t)
        {
            if (!self.try_execute(
                mefdn::forward<decltype(cmd)>(cmd)
            )) {
                break;
            }

            ++n;
        }
        
        t.commit(n);
        
        if (n > 0) {
            MEFDN_LOG_DEBUG(
                "msg:Dequeued IBV requests.\t"
                "n:{}"
            ,   n
            );
        }
        else {
            MEFDN_LOG_VERBOSE(
                "msg:No IBV requests are dequeued."
            );
        }
        
        return true;
    }
    
    mefdn::atomic<bool>    finished_;
    thread_type             th_;
};
#endif

} // namespace mecom
} // namespace menps

