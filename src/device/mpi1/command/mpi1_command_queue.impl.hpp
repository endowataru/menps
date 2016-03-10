
#pragma once

#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "device/mpi/command/mpi_completer.hpp"
#include "common/command/basic_command_queue.hpp"
#include <mgbase/basic_active_object.hpp>
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/scoped_enum.hpp>

namespace mgcom {
namespace mpi1 {

#define DEFINE_ENUM(x)  x

MGBASE_SCOPED_ENUM_DECLARE_BEGIN(mpi1_command_code)
{
    MGCOM_BASIC_COMMAND_CODES(DEFINE_ENUM)
,   MGCOM_MPI_COMMAND_CODES(DEFINE_ENUM)
}
MGBASE_SCOPED_ENUM_DECLARE_END(mpi1_command_code)

#undef DEFINE_ENUM

union mpi1_command_parameters
{
    basic_command_parameters    basic;
    mpi::mpi_command_parameters mpi;
};

struct mpi1_command
{
    mpi1_command_code       code;
    mpi1_command_parameters params;
};

class mpi1_command_queue
    : public mgbase::basic_active_object<mpi1_command_queue, mpi1_command>
    , public basic_command_queue<mpi1_command_queue, mpi1_command_code>
    , public mpi::mpi_command_queue_base<mpi1_command_queue, mpi1_command_code>
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
    
    bool try_enqueue_basic(
        const mpi1_command_code             code
    ,   const basic_command_parameters&     params
    ) {
        mpi1_command_parameters mpi1_params;
        mpi1_params.basic = params;
        
        const mpi1_command cmd = { code, mpi1_params };
        return queue_.try_push(cmd);
    }
    
    bool try_enqueue_mpi(
        const mpi1_command_code             code
    ,   const mpi::mpi_command_parameters&  params
    ) {
        mpi1_command_parameters mpi1_params;
        mpi1_params.mpi = params;
        
        const mpi1_command cmd = { code, mpi1_params };
        return queue_.try_push(cmd);
    }
    
private:
    friend class mgbase::basic_active_object<mpi1_command_queue, mpi1_command>;
    
    void process()
    {
        // Check the queue.
        if (mpi1_command* const cmd = queue_.peek())
        {
            // Call the closure.
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
    
    MGBASE_ALWAYS_INLINE bool execute(const mpi1_command& cmd)
    {
        #define CASE(x)     case mpi1_command_code::x
        #define RETURN(x)   return x;
        
        switch (mgbase::native_value(cmd.code))
        {
            MGCOM_BASIC_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.params.basic)
            MGCOM_MPI_COMMAND_EXECUTE_CASES(CASE, RETURN, cmd.params.mpi, completer_)
            
            MGBASE_COVERED_SWITCH()
        }
        
        #undef CASE
        #undef RETURN
        
        MGBASE_UNREACHABLE();
        return false;
    }
    
private:
    mpi::mpi_completer                                      completer_;
    mgbase::mpsc_circular_buffer<mpi1_command, queue_size>  queue_;
};

} // namespace mpi1
} // namespace mgcom

