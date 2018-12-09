
#include "command_consumer.hpp"
#include "fjmpi_completer.hpp"
#include "device/fjmpi/command/execute.hpp"
#include <menps/mefdn/thread.hpp>

namespace menps {
namespace mecom {
namespace fjmpi {

class command_consumer::impl
{
public:
    impl(endpoint& ep, command_queue& queue)
        : finished_{false}
        , completer_()
        , fjmpi_comp_{ep}
    {
        completer_.initialize();
        
        th_ = mefdn::thread(starter{*this, queue});
    }
    
    virtual ~impl()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        // Join the running thread.
        th_.join();
        
        completer_.finalize();
    }
    
    impl(const impl&) = delete;
    impl& operator = (const impl&) = delete;
    
    mpi::mpi_completer& get_mpi_completer() noexcept {
        return completer_;
    }
    
private:
    struct starter
    {
        impl& self;
        command_queue& queue;
        
        void operator() () {
            self.loop(queue);
        }
    };
    
    void loop(command_queue& queue)
    {
        while (MEFDN_LIKELY( !finished_ ))
        {
            // Check the queue.
            auto t = queue.try_dequeue(1);
            
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
            
            #if 0
            
            // Check the queue.
            if (command* const cmd = queue.peek())
            {
                // Call the closure.
                const bool succeeded = execute(*cmd);
                
                if (MEFDN_LIKELY(succeeded)) {
                    MEFDN_LOG_DEBUG("msg:Operation succeeded.");
                    queue.pop();
                }
                else {
                    MEFDN_LOG_DEBUG("msg:Operation failed. Postponed.");
                }
            }
            
            #endif
            
            fjmpi_comp_.poll_on_this_thread();
            
            // Do polling.
            completer_.poll_on_this_thread();
        }
    }
    
    MEFDN_NODISCARD
    bool execute(const command& cmd)
    {
        #define CASE(x)     case command_code::x:
        #define RETURN(x)   return x;
        
        switch (cmd.code)
        {
            case command_code::call:
            {
                const bool ret = cmd.func(cmd.arg);
                return ret;
            }
            
            MECOM_FJMPI_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.arg, fjmpi_comp_)
        }
        
        #undef CASE
        #undef RETURN
        
        MEFDN_UNREACHABLE();
    }
    
    bool finished_;
    mefdn::thread th_;
    
    mpi::mpi_completer completer_;
    fjmpi_completer fjmpi_comp_;
};


command_consumer::command_consumer(endpoint& ep)
    : impl_{mefdn::make_unique<impl>(ep, static_cast<command_queue&>(*this))} { }

command_consumer::~command_consumer() = default;

mpi::mpi_completer& command_consumer::get_mpi_completer() noexcept {
    return impl_->get_mpi_completer();
}

} // namespace fjmpi
} // namespace mecom
} // namespace menps

