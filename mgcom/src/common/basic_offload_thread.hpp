
#pragma once

#include <mgcom/common.hpp>

#ifdef MGCOM_FORK_EXECUTOR_THREAD
    #include <mgult/offload/basic_fork_offload_thread.hpp>
#else
    #include <mgult/offload/basic_cv_offload_thread.hpp>
#endif

#if 0
#include <mgbase/atomic.hpp>
#include <mgbase/crtp_base.hpp>
#include <mgbase/profiling/clock.hpp>
#endif

namespace mgcom {

template <typename Policy>
class basic_offload_thread
#ifdef MGCOM_FORK_EXECUTOR_THREAD
    : public mgult::basic_fork_offload_thread<Policy>
#else
    : public mgult::basic_cv_offload_thread<Policy>
#endif
{ };

#if 0

template <typename Policy>
class basic_offload_thread
{
    MGBASE_POLICY_BASED_CRTP(Policy)
    
    typedef typename Policy::thread_type    thread_type;
    
protected:
    basic_offload_thread()
        : finished_{false}
    { }
    
    void start()
    {
        #ifdef MGCOM_FORK_EXECUTOR_THREAD
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
        
        #ifndef MGCOM_FORK_EXECUTOR_THREAD
        th_.join();
        #endif
    }
    
private:
    #ifdef MGCOM_FORK_EXECUTOR_THREAD
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
                    ult::this_thread::yield();
                }
                else {
                #endif
                    if (MGBASE_UNLIKELY(
                        ! self.has_remaining()
                    )) {
                        if (MGBASE_UNLIKELY(
                            self.try_sleep()
                        )) {
                            ult::this_thread::detach();
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
        
        #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
        auto old_clock = mgbase::get_cpu_clock();
        #endif
        
        while (MGBASE_LIKELY(
            ! finished_.load(mgbase::memory_order_relaxed)
        )) {
            if (MGBASE_UNLIKELY(
                ! this->dequeue_some()
            )) {
                if (MGBASE_UNLIKELY(
                    ! self.has_remaining()
                )) {
                    #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
                    const auto cur_clock = mgbase::get_cpu_clock();
                    if (cur_clock - old_clock < MGCOM_EXECUTOR_LIFETIME_CYCLES) {
                        ult::this_thread::yield();
                    }
                    else {
                    #endif
                        if (MGBASE_UNLIKELY(
                            ! self.try_sleep()
                        )) {
                            ult::this_thread::yield();
                        }
                    #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
                    }
                    #endif
                } else {
                    ult::this_thread::yield();
                }
            }
            #if MGCOM_EXECUTOR_LIFETIME_CYCLES > 0
            else {
                old_clock = mgbase::get_cpu_clock();
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
        
        mgbase::size_t n = 0;
        
        // TODO: exception safety
        
        MGBASE_RANGE_BASED_FOR(auto&& cmd, t)
        {
            if (!self.try_execute(
                mgbase::forward<decltype(cmd)>(cmd)
            )) {
                break;
            }

            ++n;
        }
        
        t.commit(n);
        
        if (n > 0) {
            MGBASE_LOG_DEBUG(
                "msg:Dequeued IBV requests.\t"
                "n:{}"
            ,   n
            );
        }
        else {
            MGBASE_LOG_VERBOSE(
                "msg:No IBV requests are dequeued."
            );
        }
        
        return true;
    }
    
    mgbase::atomic<bool>    finished_;
    thread_type             th_;
};
#endif

} // namespace mgcom

