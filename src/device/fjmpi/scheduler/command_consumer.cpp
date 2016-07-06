
#include "command_consumer.hpp"
#include "fjmpi_completer.hpp"
#include "device/fjmpi/command/execute.hpp"
#include <mgbase/thread.hpp>

namespace mgcom {
namespace fjmpi {

class command_consumer::impl
{
public:
    impl(command_queue& queue)
        : finished_{false}
        , completer_()
        , fjmpi_comp_{}
    {
        completer_.initialize();
        
        th_ = mgbase::thread(starter{*this, queue});
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
    
    mpi::mpi_completer& get_mpi_completer() MGBASE_NOEXCEPT {
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
        while (MGBASE_LIKELY(! finished_))
        {
            // Check the queue.
            if (command* const cmd = queue.peek())
            {
                // Call the closure.
                const bool succeeded = execute(*cmd);
                
                if (MGBASE_LIKELY(succeeded)) {
                    MGBASE_LOG_DEBUG("msg:Operation succeeded.");
                    queue.pop();
                }
                else {
                    MGBASE_LOG_DEBUG("msg:Operation failed. Postponed.");
                }
            }
            
            fjmpi_comp_.poll_on_this_thread();
            
            // Do polling.
            completer_.poll_on_this_thread();
        }
    }
    
    MGBASE_WARN_UNUSED_RESULT
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
            
            MGCOM_FJMPI_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.arg, fjmpi_comp_)
        }
        
        #undef CASE
        #undef RETURN
        
        MGBASE_UNREACHABLE();
    }
    
    bool finished_;
    mgbase::thread th_;
    
    mpi::mpi_completer completer_;
    fjmpi_completer fjmpi_comp_;
};


command_consumer::command_consumer()
    : impl_{mgbase::make_unique<impl>(static_cast<command_queue&>(*this))} { }

command_consumer::~command_consumer() = default;

mpi::mpi_completer& command_consumer::get_mpi_completer() MGBASE_NOEXCEPT {
    return impl_->get_mpi_completer();
}

} // namespace fjmpi
} // namespace mgcom

