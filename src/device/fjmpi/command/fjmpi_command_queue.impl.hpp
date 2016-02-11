
#pragma once

#include <mgcom/rma/try_rma.hpp>
#include "fjmpi_command.hpp"
#include "device/mpi/command/mpi_command_queue_base.hpp"
#include "common/command/basic_command_queue.hpp"
#include <mgbase/basic_active_object.hpp>

namespace mgcom {
namespace fjmpi {

namespace /*unnamed*/ {

class fjmpi_command_queue;

struct fjmpi_command
{
    fjmpi_command_code          code;
    fjmpi_command_parameters    params;
};


class fjmpi_command_queue
    : public mgbase::basic_active_object<fjmpi_command_queue, fjmpi_command>
    , public mpi::mpi_command_queue_base
{
    typedef mgbase::basic_active_object<fjmpi_command_queue, fjmpi_command> base;
    
    static const index_t queue_size = 256 * 256; // TODO
    static const int number_of_flag_patterns = 4;
    
public:
    void initialize()
    {
        send_nic_.store(0, mgbase::memory_order_relaxed);
        
        completer_.initialize();
        
        base::start();
    }
    
    void finalize()
    {
        base::stop();
        
        completer_.finalize();
    }
    
    // Thread-safe
    bool try_get(
        const int                   src_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const int                   flags
    ,   const mgbase::operation&    on_complete
    ) {
        const fjmpi_command_parameters::contiguous_parameters params = {
            src_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   flags
        ,   on_complete
        };
        
        fjmpi_command_parameters fjmpi_params;
        fjmpi_params.contiguous = params;
        
        const fjmpi_command cmd = { FJMPI_COMMAND_GET, fjmpi_params };
        
        const bool ret = queue_.try_push(cmd);
        MGBASE_LOG_DEBUG(
            "msg:{}\tsrc_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
        ,   (ret ? "Queued FJMPI_Rdma_get." : "Failed to queue FJMPI_Rdma_get.")
        ,   src_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   flags
        );
        return ret;
    }
    
    // Thread-safe
    bool try_put(
        const int                   dest_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const int                   flags
    ,   const mgbase::operation&    on_complete
    ) {
        const fjmpi_command_parameters::contiguous_parameters params = {
            dest_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   flags
        ,   on_complete
        };
        
        fjmpi_command_parameters fjmpi_params;
        fjmpi_params.contiguous = params;
        
        const fjmpi_command cmd = { FJMPI_COMMAND_PUT, fjmpi_params };
        
        const bool ret = queue_.try_push(cmd);
        MGBASE_LOG_DEBUG(
            "msg:{}\tdest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
        ,   (ret ? "Queued FJMPI_Rdma_put." : "Failed to queue FJMPI_Rdma_put.")
        ,   dest_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   flags
        );
        return ret;
    }
    
    // Thread-safe
    int select_flags()
    {
        const int result = send_nic_.fetch_add(1, mgbase::memory_order_relaxed);
        const int num = result % number_of_flag_patterns;
        
        return flag_patterns_[num];
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
        return queue_.try_push(cmd);
    }
    
private:
    friend class mgbase::basic_active_object<fjmpi_command_queue, fjmpi_command>;
    
    MGBASE_ALWAYS_INLINE fjmpi_command* peek_queue() { return queue_.peek(); }
    
    MGBASE_ALWAYS_INLINE void pop_queue() { queue_.pop(); }
    
    MGBASE_ALWAYS_INLINE bool execute(const fjmpi_command& cmd)
    {
        return execute_on_this_thread(cmd.code, cmd.params, completer_);
    }
    
    // NOT thread-safe
    MGBASE_ALWAYS_INLINE void poll()
    {
        completer_.poll_on_this_thread();
    }
    
    fjmpi_completer                                         completer_;
    mgbase::mpsc_circular_buffer<fjmpi_command, queue_size> queue_;
    
    mgbase::atomic<int> send_nic_;
    
    static const int flag_patterns_[number_of_flag_patterns];
};

const int fjmpi_command_queue::flag_patterns_[number_of_flag_patterns] = {
    FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 | FJMPI_RDMA_PATH0
,   FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1 | FJMPI_RDMA_PATH0
,   FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2 | FJMPI_RDMA_PATH0
,   FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3 | FJMPI_RDMA_PATH0
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

