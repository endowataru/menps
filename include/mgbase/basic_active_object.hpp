
#pragma once

#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgbase {

template <typename Derived, typename Closure>
class basic_active_object
{
protected:
    void start()
    {
        finished_ = false;
        
        // Start a new thread.
        th_ = mgbase::thread(starter(this));
        
        MGBASE_LOG_DEBUG("msg:Initialized active object.");
    }
    
    void stop()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        // Join the running thread.
        th_.join();
        
        MGBASE_LOG_DEBUG("msg:Finalized active object.");
    }
    
private:
    void loop()
    {
        while (MGBASE_LIKELY(!finished_))
        {
            // Check the queue.
            if (Closure* closure = derived().peek_queue())
            {
                // Call the closure.
                const bool succeeded = derived().execute(*closure);
                
                if (succeeded) {
                    MGBASE_LOG_DEBUG("msg:Operation succeeded.");
                    derived().pop_queue();
                }
                else {
                    MGBASE_LOG_DEBUG("msg:Operation failed. Postponed.");
                }
            }
            
            // Do polling.
            derived().poll();
        }
    }
    
    class starter
    {
    public:
        explicit starter(basic_active_object* self)
            : self_(self) { }
        
        void operator() ()
        {
            self_->loop();
        }
        
    private:
        basic_active_object* self_;
    };
    
          Derived& derived()       MGBASE_NOEXCEPT { return static_cast<      Derived&>(*this); }
    const Derived& derived() const MGBASE_NOEXCEPT { return static_cast<const Derived&>(*this); }
    
    bool            finished_;
    mgbase::thread  th_;
};

} // namespace mgbase

