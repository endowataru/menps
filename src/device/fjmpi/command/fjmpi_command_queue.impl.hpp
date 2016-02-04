
#pragma once

#include <mgcom/rma/try_rma.hpp>
#include "fjmpi_command.hpp"
#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "common/rma/request_queue.hpp"

namespace mgcom {
namespace fjmpi {

namespace /*unnamed*/ {

class fjmpi_command_queue;

struct fjmpi_command
{
    fjmpi_command_code          code;
    fjmpi_command_parameters    params;
    
    MGBASE_ALWAYS_INLINE bool execute(fjmpi_command_queue* queue) const;
};


class fjmpi_command_queue
    : public mpi::mpi_command_queue_base
{
    static const index_t queue_size = 256 * 256; // TODO
    
public:
    void initialize()
    {
        completer_.initialize();
        queue_.initialize(this);
        
        send_nic_.store(0, mgbase::memory_order_relaxed);
    }
    
    void finalize()
    {
        queue_.finalize();
        completer_.initialize();
    }
    
    // Thread-safe
    bool try_get(
        const int                   src_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const int                   extra_flags
    ,   const mgbase::operation&    on_complete
    ) {
        const int nic = select_nic();
        
        const fjmpi_command_parameters::contiguous_parameters params = {
            src_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   nic
        ,   extra_flags
        ,   on_complete
        };
        
        fjmpi_command_parameters fjmpi_params;
        fjmpi_params.contiguous = params;
        
        const fjmpi_command cmd = { FJMPI_COMMAND_GET, fjmpi_params };
        
        const bool ret = queue_.try_enqueue(cmd);
        MGBASE_LOG_DEBUG(
            "msg:{}\tsrc_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\textra_flags:{}"
        ,   (ret ? "Queued FJMPI_Rdma_get." : "Failed to queue FJMPI_Rdma_get.")
        ,   src_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   nic
        ,   extra_flags
        );
        return ret;
    }
    
    // Thread-safe
    bool try_put(
        const int                   dest_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const int                   extra_flags
    ,   const mgbase::operation&    on_complete
    ) {
        const int nic = select_nic();
         
        const fjmpi_command_parameters::contiguous_parameters params = {
            dest_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   nic
        ,   extra_flags
        ,   on_complete
        };
        
        fjmpi_command_parameters fjmpi_params;
        fjmpi_params.contiguous = params;
        
        const fjmpi_command cmd = { FJMPI_COMMAND_PUT, fjmpi_params };
        
        const bool ret = queue_.try_enqueue(cmd);
        MGBASE_LOG_DEBUG(
            "msg:{}\tdest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\textra_flags:{}"
        ,   (ret ? "Queued FJMPI_Rdma_put." : "Failed to queue FJMPI_Rdma_put.")
        ,   dest_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   nic
        ,   extra_flags
        );
        return ret;
    }
    
    fjmpi_completer& get_completer() {
        return completer_;
    }

protected:
    virtual bool try_enqueue_mpi(
        const mpi::mpi_command_code         code
    ,   const mpi::mpi_command_parameters&  params
    ) MGBASE_OVERRIDE
    {
        fjmpi_command_parameters fjmpi_params;
        fjmpi_params.mpi1 = params;
        
        fjmpi_command cmd = {
            static_cast<fjmpi_command_code>(code)
        ,   fjmpi_params
        };
        return queue_.try_enqueue(cmd);
    }
    
private:
    friend class rma::request_queue<fjmpi_command, queue_size>;
    
    // NOT thread-safe
    void poll()
    {
        completer_.poll_on_this_thread();
    }
    
    fjmpi_completer completer_;
    rma::request_queue<fjmpi_command, queue_size> queue_;
    
    // Thread-safe
    int select_nic()
    {
        const int result = send_nic_.fetch_add(1, mgbase::memory_order_relaxed);
        return result % fjmpi_completer::max_nic_count;
    }
    
    mgbase::atomic<int> send_nic_;
};

bool fjmpi_command::execute(fjmpi_command_queue* const queue) const
{
    return execute_on_this_thread(code, params, queue->get_completer());
}

} // unnamed namespace

} // namespace rma
} // namespace mgcom

