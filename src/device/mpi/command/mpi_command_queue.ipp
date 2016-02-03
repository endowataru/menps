
#pragma once

#include "basic_mpi_command_queue.hpp"
#include "common/rma/request_queue.hpp"
#include "mpi_command_queue.hpp"
#include "mpi_completer.hpp"

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

class mpi_command_queue;

struct mpi_command
{
    mpi_command_code        code;
    mpi_command_parameters  params;
    
    MGBASE_ALWAYS_INLINE bool execute(mpi_command_queue* /*queue*/) const;
};

class mpi_command_queue
    : public basic_mpi_command_queue<mpi_command_queue>
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
    
    mpi_completer& get_completer() MGBASE_NOEXCEPT {
        return completer_;
    }
    
private:
    friend class basic_mpi_command_queue<mpi_command_queue>;
    
    bool try_enqueue(
        const mpi_command_code          code
    ,   const mpi_command_parameters&   params
    )
    {
        const mpi_command cmd = { code, params };
        return queue_.try_enqueue(cmd);
    }
    
private:
    friend class rma::request_queue<mpi_command, queue_size>;
    
    void poll()
    {
        completer_.poll_on_this_thread();
    }

private:
    rma::request_queue<mpi_command, queue_size> queue_;
    mpi_completer completer_;
};

bool mpi_command::execute(mpi_command_queue* queue) const
{
    return execute_on_this_thread(code, params, queue->get_completer());
}

} // unnamed namespace

} // namespace mpi
} // namespace mgcom

