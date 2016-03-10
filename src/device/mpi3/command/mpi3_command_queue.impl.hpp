
#pragma once

#include "mpi3_command_queue_base.hpp"
#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "common/command/basic_command_queue.hpp"
#include <mgbase/basic_active_object.hpp>
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/scoped_enum.hpp>

namespace mgcom {
namespace mpi3 {

namespace /*unnamed*/ {

#define DEFINE_ENUM(x)  x

MGBASE_SCOPED_ENUM_DECLARE_BEGIN(command_code)
{
    MGCOM_BASIC_COMMAND_CODES(DEFINE_ENUM)
,   MGCOM_MPI_COMMAND_CODES(DEFINE_ENUM)
,   MGCOM_MPI3_COMMAND_CODES(DEFINE_ENUM)
}
MGBASE_SCOPED_ENUM_DECLARE_END(command_code)

#undef DEFINE_ENUM

struct command_parameters
{
    basic_command_parameters    basic;
    mpi::mpi_command_parameters mpi;
    mpi3_command_parameters     mpi3;
};

struct mpi3_command
{
    command_code        code;
    command_parameters  params;
};

class mpi3_command_queue
    : public mgbase::basic_active_object<mpi3_command_queue, mpi3_command>
    , public basic_command_queue<mpi3_command_queue, command_code>
    , public mpi::mpi_command_queue_base<mpi3_command_queue, command_code>
    , public mpi3_command_queue_base<mpi3_command_queue, command_code>
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
    
    bool try_enqueue_basic(
        const command_code                  code
    ,   const basic_command_parameters&     basic_params
    ) {
        command_parameters params;
        params.basic = basic_params;
        
        const mpi3_command cmd = { code, params };
        return queue_.try_push(cmd);
    }
    
    bool try_enqueue_mpi(
        const command_code                  code
    ,   const mpi::mpi_command_parameters&  mpi_params
    ) {
        command_parameters params;
        params.mpi = mpi_params;
        
        const mpi3_command cmd = { code, params };
        return queue_.try_push(cmd);
    }
    
    bool try_enqueue_mpi3(
        const command_code              code
    ,   const mpi3_command_parameters&  mpi3_params
    ) {
        command_parameters params;
        params.mpi3 = mpi3_params;
        
        const mpi3_command cmd = { code, params };
        return queue_.try_push(cmd);
    }
    
private:
    friend class mgbase::basic_active_object<mpi3_command_queue, mpi3_command>;
     
    void process()
    {
        // Check the queue.
        if (mpi3_command* const cmd = queue_.peek())
        {
            // Execute the command.
            const bool succeeded = execute(*cmd);
            
            if (succeeded) {
                MGBASE_LOG_DEBUG("msg:Operation succeeded.");
                queue_.pop();
            }
            else {
                MGBASE_LOG_DEBUG("msg:Operation failed. Postponed.");
            }
        }
        
        // Do polling.
        completer_.poll_on_this_thread();
    }
    
    MGBASE_ALWAYS_INLINE bool execute(const mpi3_command& cmd)
    {
        #define CASE(x)     case command_code::x
        #define RETURN(x)   return x;
        
        switch (mgbase::native_value(cmd.code))
        {
            MGCOM_BASIC_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.params.basic)
            MGCOM_MPI_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.params.mpi, completer_.get_mpi1_completer())
            MGCOM_MPI3_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.params.mpi3, completer_)
        }
        
        #undef CASE
        #undef RETURN
        
        MGBASE_UNREACHABLE();
        return false;
    }
    
private:
    mgbase::mpsc_circular_buffer<mpi3_command, queue_size>  queue_;
    mpi3_completer                                          completer_;
};

} // unnamed namespace

} // namespace mpi3
} // namespace mgcom

