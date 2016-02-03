
#pragma once

#include <mgcom/rma/try_rma.hpp>
#include "rma_core.ipp"
#include "common/rma/request_queue.hpp"

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

class queued_rma
{
    static const index_t queue_size = 256 * 256; // TODO
    
    enum request_code
    {
        REQUEST_GET
    ,   REQUEST_PUT
    ,   REQUEST_REGISTER
    ,   REQUEST_DEREGISTER
    ,   REQUEST_GET_REMOTE_ADDR
    ,   REQUEST_LOCK
    };
    
    struct request
    {
        request_code        code;
        mgbase::operation   on_complete;
        union argument {
            struct contiguous {
                int                 proc;
                mgbase::uint64_t    laddr;
                mgbase::uint64_t    raddr;
                std::size_t         size_in_bytes;
                int                 extra_flags;
            }
            cont;
            
            struct registration {
                void*               buf;
                std::size_t         length;
                mgbase::uint64_t*   laddr_result;
                int*                memid_result;
            }
            reg;
            
            struct deregistration {
                int memid;
            }
            dereg;
            
            struct get_remote_addr {
                int                 pid;
                int                 memid;
                mgbase::uint64_t*   raddr_result;
            }
            get_raddr;
            
            struct locking {
            }
            lc;
        }
        arg;
        
        bool operator() (queued_rma* self)
        {
            switch (code)
            {
                case REQUEST_GET: {
                    return self->core_.try_get(
                        arg.cont.proc
                    ,   arg.cont.laddr
                    ,   arg.cont.raddr
                    ,   arg.cont.size_in_bytes
                    ,   this->on_complete
                    ,   arg.cont.extra_flags
                    ,   self->select_nic()
                    );
                }
                
                case REQUEST_PUT: {
                    return self->core_.try_put(
                        arg.cont.proc
                    ,   arg.cont.laddr
                    ,   arg.cont.raddr
                    ,   arg.cont.size_in_bytes
                    ,   this->on_complete
                    ,   arg.cont.extra_flags
                    ,   self->select_nic()
                    );
                }
                
                case REQUEST_REGISTER: {
                    const int memid
                        = self->core_.register_memory(
                            arg.reg.buf
                        ,   arg.reg.length
                        ,   arg.reg.laddr_result
                        );
                    
                    *arg.reg.memid_result = memid;
                    
                    mgbase::execute(this->on_complete); // this method executes
                    
                    return true;
                }
                
                case REQUEST_DEREGISTER: {
                    self->core_.deregister_memory(
                        arg.dereg.memid
                    );
                    
                    mgbase::execute(this->on_complete); // this method executes
                    
                    return true;
                }
                
                case REQUEST_GET_REMOTE_ADDR: {
                    const mgbase::uint64_t raddr
                        = self->core_.get_remote_addr(
                            arg.get_raddr.pid
                        ,   arg.get_raddr.memid
                        );
                    
                    *arg.get_raddr.raddr_result = raddr;
                    
                    mgbase::execute(this->on_complete); // this method executes
                    return true;
                }
                
                case REQUEST_LOCK: {
                    MGBASE_ASSERT(self->lock_request_flag_.load()); // true
                    MGBASE_ASSERT(!self->lock_reply_flag_.load());  // false
                    
                    self->lock_reply_flag_.store(true, mgbase::memory_order_release);
                    
                    MGBASE_LOG_DEBUG("msg:Communication thread was locked by other thread.");
                    // TODO: Use mutex
                    MGBASE_ASM_COMMENT("communication thread is locked");
                    while (self->lock_request_flag_.load(mgbase::memory_order_acquire)) {
                        //MGBASE_LOG_VERBOSE("msg:Communication thread is still locked.");
                    }
                    MGBASE_ASM_COMMENT("communication thread is unlocked");
                    
                    MGBASE_ASSERT(!self->lock_request_flag_.load()); // false
                    MGBASE_ASSERT(self->lock_reply_flag_.load());    // true
                    
                    self->lock_reply_flag_.store(false, mgbase::memory_order_release);
                    
                    MGBASE_LOG_DEBUG("msg:Communication thread was unlocked.");
                    return true;
                }
                
                default:
                    MGBASE_UNREACHABLE();
                    fjmpi_error::emit();
            }
        }
    };
    
public:
    void initialize()
    {
        core_.initialize();
        queue_.initialize(this);
    }
    
    void finalize()
    {
        queue_.finalize();
        core_.finalize();
    }
    
    // Thread-safe
    bool try_put(
        const int                   dest_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const mgbase::operation&    on_complete
    ,   const int                   extra_flags
    ) {
        request req;
        req.code        = REQUEST_PUT;
        req.on_complete = on_complete;
        
        request::argument::contiguous& arg = req.arg.cont;
        arg.proc            = dest_proc;
        arg.raddr           = raddr;
        arg.laddr           = laddr;
        arg.size_in_bytes   = size_in_bytes;
        arg.extra_flags     = extra_flags;
        
        const bool ret = queue_.try_enqueue(req);
        MGBASE_LOG_DEBUG(
            "msg:{}\tdest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\textra_flags:{}"
        ,   (ret ? "Queued RDMA Put." : "Failed to queue RDMA Put.")
        ,   dest_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   extra_flags
        );
        return ret;
    }
    
    // Thread-safe
    bool try_get(
        const int                   src_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const mgbase::operation&    on_complete
    ,   const int                   extra_flags
    ) {
        request req;
        req.code        = REQUEST_GET;
        req.on_complete = on_complete;
        
        request::argument::contiguous& arg = req.arg.cont;
        arg.proc            = src_proc;
        arg.raddr           = raddr;
        arg.laddr           = laddr;
        arg.size_in_bytes   = size_in_bytes;
        arg.extra_flags     = extra_flags;
        
        const bool ret = queue_.try_enqueue(req);
        MGBASE_LOG_DEBUG(
            "msg:{}\tsrc_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\textra_flags:{}"
        ,   (ret ? "Queued RDMA Get." : "Failed to queue RDMA Get.")
        ,   src_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   extra_flags
        );
        return ret;
    }
    
    // Thread-safe
    bool try_register_memory(
        void* const                 buf
    ,   const std::size_t           length
    ,   mgbase::uint64_t* const     laddr_result
    ,   int* const                  memid_result
    ,   const mgbase::operation&    on_complete
    ) {
        request req;
        req.code        = REQUEST_REGISTER;
        req.on_complete = on_complete;
        
        request::argument::registration& arg = req.arg.reg;
        arg.buf             = buf;
        arg.length          = length;
        arg.laddr_result    = laddr_result;
        arg.memid_result    = memid_result;
        
        const bool ret = queue_.try_enqueue(req);
        MGBASE_LOG_DEBUG(
            "msg:{}\tptr:{:x}\tsize:{}\tladdr_result:{:x}\tmemid_result:{:x}"
        ,   (ret ? "Queued memory registration." : "Failed to queue memory registraion.")
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   length
        ,   reinterpret_cast<mgbase::uintptr_t>(laddr_result)
        ,   reinterpret_cast<mgbase::uintptr_t>(memid_result)
        );
        return ret;
    }
    
    // Thread-safe
    bool try_deregister_memory(
        const int                   memid
    ,   const mgbase::operation&    on_complete
    ) {
        request req;
        req.code        = REQUEST_DEREGISTER;
        req.on_complete = on_complete;
        
        request::argument::deregistration& arg = req.arg.dereg;
        arg.memid = memid;
        
        const bool ret = queue_.try_enqueue(req);
        MGBASE_LOG_DEBUG(
            "msg:{}\tmemid:{}"
        ,   (ret ? "Queued memory deregistration." : "Failed to queue memory deregistraion.")
        ,   memid
        );
        return ret;
    }
    
    // Thread-safe
    bool try_get_remote_addr(
        int const                   pid
    ,   int const                   memid
    ,   mgbase::uint64_t* const     raddr_result
    ,   const mgbase::operation&    on_complete
    )
    {
        request req;
        req.code        = REQUEST_GET_REMOTE_ADDR;
        req.on_complete = on_complete;
        
        request::argument::get_remote_addr& arg = req.arg.get_raddr;
        arg.pid             = pid;
        arg.memid           = memid;
        arg.raddr_result    = raddr_result;
        
        const bool result = queue_.try_enqueue(req);
        MGBASE_LOG_DEBUG(
            "msg:{}\tproc:{}\tmemid:{}\traddr_result:{:x}"
        ,   (result ? "Queued getting remote address." : "Failed to queue getting remote address.")
        ,   pid
        ,   memid
        ,   reinterpret_cast<mgbase::uintptr_t>(raddr_result)
        );
        return result;
    }
    
    // Thread-safe
    bool try_lock()
    {
        bool expected = false;
        if (!lock_flag_.compare_exchange_strong(
            expected
        ,   true
        ,   mgbase::memory_order_acquire
        ,   mgbase::memory_order_acquire)
        ) {
            MGBASE_LOG_DEBUG("msg:Failed to lock MPI.");
            
            return false;
        }
        
        // Initialize the reply flag.
        lock_request_flag_.store(true);
        lock_reply_flag_.store(false);
        
        request req;
        req.code = REQUEST_LOCK;
        
        if (queue_.try_enqueue(req))
        {
            // Enqueued.
            
            // Wait for the reply flag.
            while (!lock_reply_flag_.load(mgbase::memory_order_acquire))
            {
                //MGBASE_ASSERT(!queue_.empty());
                
                MGBASE_LOG_DEBUG("msg:Waiting for communication thread to lock MPI.");
            }
            
            MGBASE_LOG_DEBUG("msg:Locked MPI.");
            
            return true;
        }
        else {
            MGBASE_LOG_DEBUG("msg:Failed to enqueue MPI lock. MPI lock failed.");
            
            // Unlock.
            lock_flag_.store(false, mgbase::memory_order_release);
            
            return false;
        }
    }
    
    // Thread-safe
    void unlock()
    {
        MGBASE_ASSERT(lock_flag_.load());
        
        MGBASE_ASSERT(lock_request_flag_.load());
        MGBASE_ASSERT(lock_reply_flag_.load());
        
        lock_request_flag_.store(false, mgbase::memory_order_release);
        
        // Wait for the reply flag.
        while (lock_reply_flag_.load(mgbase::memory_order_acquire))
        {
            MGBASE_LOG_DEBUG("msg:Waiting for communication thread to lock MPI.");
        }
        
        MGBASE_ASSERT(!lock_request_flag_.load());
        MGBASE_ASSERT(!lock_reply_flag_.load());
        
        lock_flag_.store(false, mgbase::memory_order_release);
        
        MGBASE_LOG_DEBUG("msg:Unlocked MPI.");
    }
    

private:
    friend class request_queue<request, queue_size>;
    
    // NOT thread-safe
    void poll()
    {
        int pid;
        int tag;
        core_.poll_nic(poll_nic_, &pid, &tag);
        
        if (++poll_nic_ >= rma_core::max_nic_count)
            poll_nic_ = 0;
    }
    
    rma_core core_;
    request_queue<request, queue_size> queue_;
    
    int select_nic()
    {
        const int result = send_nic_;
        
        if (++send_nic_ >= rma_core::max_nic_count)
            send_nic_ = 0;
        
        return result;
    }
    
    int poll_nic_;
    int send_nic_;
    mgbase::atomic<bool> lock_flag_;
    mgbase::atomic<bool> lock_request_flag_; // TODO: cache line size
    mgbase::atomic<bool> lock_reply_flag_;
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

