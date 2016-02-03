
#pragma once

#include "mpi3_command_queue_base.hpp"
#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "common/rma/request_queue.hpp"

namespace mgcom {
namespace mpi3 {

namespace /*unnamed*/ {

class mpi3_command_queue;

struct mpi3_command
{
    mpi3_command_code       code;
    mpi3_command_parameters params;
    
    MGBASE_ALWAYS_INLINE bool execute(mpi3_command_queue* queue) const;
};

class mpi3_command_queue
    : public mpi::mpi_command_queue_base
    , public mpi3_command_queue_base
{
    static const index_t queue_size = 196; // TODO
    
public:
    void initialize()
    {
        completer_.initialize();
        queue_.initialize(this);
    }
    void finalize()
    {
        queue_.finalize();
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
        
        return queue_.try_enqueue(cmd);
    }
    
    virtual bool try_enqueue_mpi3(
        const mpi3_command_code         code
    ,   const mpi3_command_parameters&  params
    ) MGBASE_OVERRIDE
    {
        const mpi3_command cmd = { code, params };
        return queue_.try_enqueue(cmd);
    }
    
private:
    friend class rma::request_queue<mpi3_command, queue_size>;
    
    void poll()
    {
        completer_.poll_on_this_thread();
    }
    
private:
    rma::request_queue<mpi3_command, queue_size> queue_;
    mpi3_completer completer_;
};

bool mpi3_command::execute(mpi3_command_queue* queue) const
{
    return execute_on_this_thread(code, params, queue->get_completer());
}

} // unnamed namespace

} // namespace mpi3
} // namespace mgcom

