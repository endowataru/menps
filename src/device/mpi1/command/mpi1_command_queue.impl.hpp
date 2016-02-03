
#pragma once

#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "device/mpi/command/mpi_completer.hpp"
#include "common/rma/request_queue.hpp"

namespace mgcom {
namespace mpi1 {

class mpi1_command_queue;

struct mpi1_command
{
    mpi::mpi_command_code       code;
    mpi::mpi_command_parameters params;
    
    MGBASE_ALWAYS_INLINE bool execute(mpi1_command_queue* /*queue*/) const;
};

class mpi1_command_queue
    : public mpi::mpi_command_queue_base
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
    
    mpi::mpi_completer& get_completer() MGBASE_NOEXCEPT {
        return completer_;
    }
    
private:
    virtual bool try_enqueue_mpi(
        const mpi::mpi_command_code          code
    ,   const mpi::mpi_command_parameters&   params
    ) MGBASE_OVERRIDE
    {
        const mpi1_command cmd = { code, params };
        return queue_.try_enqueue(cmd);
    }
    
private:
    friend class rma::request_queue<mpi1_command, queue_size>;
    
    void poll()
    {
        completer_.poll_on_this_thread();
    }

private:
    rma::request_queue<mpi1_command, queue_size> queue_;
    mpi::mpi_completer completer_;
};

bool mpi1_command::execute(mpi1_command_queue* queue) const
{
    return execute_on_this_thread(code, params, queue->get_completer());
}


} // namespace mpi1
} // namespace mgcom

