
#pragma once

#include <mgbase/atomic.hpp>
#include <mgbase/crtp_base.hpp>

namespace mgcom {

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
        th_ = thread_type(starter{*this});
    }
    
public:
    ~basic_offload_thread() /*noexcept*/
    {
        auto& self = this->derived();
        
        finished_.store(true);
        
        self.force_notify();
        
        th_.join();
    }
    
private:
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
        
        while (MGBASE_LIKELY(
            ! finished_.load(mgbase::memory_order_relaxed)
        )) {
            if (MGBASE_UNLIKELY(
                ! this->dequeue_some()
            )) {
                if (MGBASE_UNLIKELY(
                    ! self.has_remaining()
                )) {
                    if (MGBASE_UNLIKELY(
                        ! self.try_sleep()
                    )) {
                        ult::this_thread::yield();
                    }
                }
            }
            
            self.post_all();
        }
    }
    
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

} // namespace mgcom

