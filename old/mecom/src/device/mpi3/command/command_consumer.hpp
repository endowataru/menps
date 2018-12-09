
#pragma once

#include "command_queue.hpp"
#include "device/mpi/command/mpi_completer.hpp"
#include "device/mpi3/rma/rma_window.hpp"
#include <menps/mecom/ult.hpp>

namespace menps {
namespace mecom {
namespace mpi3 {

class command_consumer
    : public virtual command_queue
{
public:
    command_consumer()
        : finished_{false}
        , completer_()
    {
        completer_.initialize();
        
        th_ = ult::thread(starter{*this});
    }
    
    virtual ~command_consumer()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        // Join the running thread.
        th_.join();
        
        completer_.finalize();
    }
    
    command_consumer(const command_consumer&) = delete;
    command_consumer& operator = (const command_consumer&) = delete;
    
    rma_window& get_win() noexcept
    {
        return win_;
    }
    
protected:
    mpi::mpi_completer& get_completer() noexcept
    {
        return completer_;
    }
    
private:
    struct starter
    {
        command_consumer& self;
        
        void operator() () {
            self.loop();
        }
    };
    
    void loop()
    {
        while (MEFDN_LIKELY( !finished_ ))
        {
            // Check the queue.
            auto t = this->try_dequeue(1);
            
            if (t.valid())
            {
                const auto& cmd = *t.begin();
                
                // Call the closure.
                const bool succeeded = execute(cmd);
                
                if (MEFDN_LIKELY(succeeded)) {
                    MEFDN_LOG_DEBUG("msg:Operation succeeded.");
                    t.commit(1);
                }
                else {
                    MEFDN_LOG_DEBUG("msg:Operation failed. Postponed.");
                    t.commit(0);
                }
            }
            
            // Do polling.
            completer_.poll_on_this_thread();
        }
    }
    
    MEFDN_NODISCARD
    bool execute(const command& cmd)
    {
        return cmd.func(cmd.arg);
    }
    
    bool finished_;
    ult::thread th_;
    
    mpi::mpi_completer completer_;
    
    rma_window win_;
};

} // namespace mpi3
} // namespace mecom
} // namespace menps

