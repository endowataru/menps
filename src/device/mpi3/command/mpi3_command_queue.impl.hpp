
#pragma once

#include "mpi3_command_queue_base.hpp"
#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "common/command/basic_command_queue.hpp"
#include <mgbase/basic_active_object.hpp>

namespace mgcom {
namespace mpi3 {

namespace /*unnamed*/ {

struct mpi3_command
{
    mpi3_command_code       code;
    mpi3_command_parameters params;
};

class mpi3_command_queue
    : public mgbase::basic_active_object<mpi3_command_queue, mpi3_command>
    , public mpi::mpi_command_queue_base
    , public mpi3_command_queue_base
{
    typedef mgbase::basic_active_object<mpi3_command_queue, mpi3_command>   base;
    
    static const index_t queue_size = 196; // TODO
    
public:
    void initialize()
    {
        completer_.initialize();
        
        base::start();
    }
    void finalize()
    {
        base::stop();
        
        completer_.finalize();
    }
    
    mpi3_completer& get_completer() MGBASE_NOEXCEPT {
        return completer_;
    }
    
private:
    virtual bool try_enqueue_mpi(
        const mpi::mpi_command_code         code
    ,   const mpi::mpi_command_parameters&  params
    ) MGBASE_OVERRIDE
    {
        mpi3_command cmd;
        cmd.code = static_cast<mpi3_command_code>(code);
        cmd.params.mpi1 = params;
        
        return queue_.try_push(cmd);
    }
    
    virtual bool try_enqueue_mpi3(
        const mpi3_command_code         code
    ,   const mpi3_command_parameters&  params
    ) MGBASE_OVERRIDE
    {
        const mpi3_command cmd = { code, params };
        return queue_.try_push(cmd);
    }
    
private:
    friend class mgbase::basic_active_object<mpi3_command_queue, mpi3_command>;
     
    void process()
    {
        // Check the queue.
        if (mpi3_command* closure = peek_queue())
        {
            // Call the closure.
            const bool succeeded = execute(*closure);
            
            if (succeeded) {
                MGBASE_LOG_DEBUG("msg:Operation succeeded.");
                pop_queue();
            }
            else {
                MGBASE_LOG_DEBUG("msg:Operation failed. Postponed.");
            }
        }
        
        // Do polling.
        poll();
    }
    
    mpi3_command* peek_queue() { return queue_.peek(); }
    
    void pop_queue() { queue_.pop(); }
    
    MGBASE_ALWAYS_INLINE bool execute(const mpi3_command& cmd)
    {
        return execute_on_this_thread(cmd.code, cmd.params, completer_);
    }
    
    MGBASE_ALWAYS_INLINE void poll()
    {
        completer_.poll_on_this_thread();
    }
    
private:
    mgbase::mpsc_circular_buffer<mpi3_command, queue_size>  queue_;
    mpi3_completer                                          completer_;
};

} // unnamed namespace

} // namespace mpi3
} // namespace mgcom

