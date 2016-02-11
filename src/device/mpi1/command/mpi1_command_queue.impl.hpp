
#pragma once

#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "device/mpi/command/mpi_completer.hpp"
#include "common/command/basic_command_queue.hpp"
#include <mgbase/basic_active_object.hpp>

namespace mgcom {
namespace mpi1 {

struct mpi1_command
{
    mpi::mpi_command_code       code;
    mpi::mpi_command_parameters params;
};

class mpi1_command_queue
    : public mgbase::basic_active_object<mpi1_command_queue, mpi1_command>
    , public mpi::mpi_command_queue_base
{
    typedef mgbase::basic_active_object<mpi1_command_queue, mpi1_command> base;
    
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

protected:
    virtual bool try_enqueue_mpi(
        const mpi::mpi_command_code         code
    ,   const mpi::mpi_command_parameters&  params
    ) MGBASE_OVERRIDE
    {
        const mpi1_command cmd = { code, params };
        return queue_.try_push(cmd);
    }
    
private:
    friend class mgbase::basic_active_object<mpi1_command_queue, mpi1_command>;
    
    mpi1_command* peek_queue() { return queue_.peek(); }
    
    void pop_queue() { queue_.pop(); }
    
    MGBASE_ALWAYS_INLINE bool execute(const mpi1_command& cmd)
    {
        return mpi::execute_on_this_thread(cmd.code, cmd.params, completer_);
    }
    
    MGBASE_ALWAYS_INLINE void poll()
    {
        completer_.poll_on_this_thread();
    }

private:
    mpi::mpi_completer                                      completer_;
    mgbase::mpsc_circular_buffer<mpi1_command, queue_size>  queue_;
};


} // namespace mpi1
} // namespace mgcom

