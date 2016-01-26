
#pragma once

#include "device/mpi/mpi_base.hpp"

#include "mpi-ext.h"

#include <algorithm>

//#include <mgbase/lockfree/static_bounded_index_queue.hpp>
#include <mgbase/lockfree/mpmc_bounded_queue.hpp>

#include "device/fjmpi/fjmpi_error.hpp"

#include <mgbase/logging/logger.hpp>

#include <queue> // TODO

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

/*
 * This class is not thread-safe.
 */

class rma_core
{
    static const int max_memid_count = 510;
    static const int max_tag_count = 15;
   
public:
    static const int max_nic_count = 4;
    
    rma_core()
        : memid_queue_(max_tag_count + 1) { }
    
    void initialize()
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_init()
        );
        
        info_by_procs_ = new processor_info[number_of_processes()];
        
        for (int memid = 0; memid < max_memid_count; memid++)
            memid_queue_.enqueue(memid);
        
        MGBASE_LOG_DEBUG("msg:Initialized FJMPI.");
    }
    
    void finalize()
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_finalize()
        );
        
        delete[] info_by_procs_;
        
        MGBASE_LOG_DEBUG("msg:Finalized FJMPI.");
    }
    
    int register_memory(void* const buf, const std::size_t length, mgbase::uint64_t* const laddr_result)
    {
        int memid;
        if (!memid_queue_.dequeue(memid)) {
            MGBASE_LOG_WARN("Exceeded the capacity of memid");
            throw fjmpi_error(); // exceeded the capacity of memid
        }
        
        const mgbase::uint64_t address =
            fjmpi_error::assert_not_error(
                FJMPI_Rdma_reg_mem(memid, buf, length)
            );
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region.\tptr:{:x}\tsize:{}\tladdr:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   length
        ,   address
        );
        
        *laddr_result = address;
        return memid;
    }
    
    mgbase::uint64_t get_remote_addr(const int pid, const int memid)
    {
        const mgbase::uint64_t raddr =
            fjmpi_error::assert_not_error(
                FJMPI_Rdma_get_remote_addr(pid, memid)
            );
        
        MGBASE_LOG_DEBUG(
            "msg:Got remote address.\tproc:{}\tmemid:{}\traddr:{:x}"
        ,   pid
        ,   memid
        ,   raddr
        );
        
        return raddr;
    }
    
    void deregister_memory(const int memid)
    {
        fjmpi_error::assert_zero(
            FJMPI_Rdma_dereg_mem(memid)
        );
        
        memid_queue_.enqueue(memid);
        
        MGBASE_LOG_DEBUG(
            "msg:Deregistered region.\tmemid:{}"
        ,   memid
        );
    }
    
    bool try_put(
        const int                   dest_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const mgbase::operation&    on_complete
    ,   const int                   extra_flags
    ,   const int                   nic
    ) {
        const int flags = nic | extra_flags;
        
        int tag;
        const bool found_tag = try_new_tag(dest_proc, nic, &tag);
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "dest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
        ,   (found_tag ? "RDMA Put." : "RDMA Put failed because tag capacity exceeded.")
        ,   dest_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   flags
        );
        
        if (MGBASE_UNLIKELY(!found_tag))
            return false;
        
        fjmpi_error::assert_zero(
            FJMPI_Rdma_put(dest_proc, tag, raddr, laddr, size_in_bytes, flags)
        );
        
        set_notification(dest_proc, nic, tag, on_complete);
        
        return true;
    }
    
    bool try_get(
        const int                   src_proc
    ,   const mgbase::uint64_t      laddr
    ,   const mgbase::uint64_t      raddr
    ,   const std::size_t           size_in_bytes
    ,   const mgbase::operation&    on_complete
    ,   const int                   extra_flags
    ,   const int                   nic
    ) {
        const int flags = nic | extra_flags;
        
        int tag;
        const bool found_tag = try_new_tag(src_proc, nic, &tag);
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "src_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
        ,   (found_tag ? "RDMA Get." : "RDMA Get failed because tag capacity exceeded.")
        ,   src_proc
        ,   laddr
        ,   raddr
        ,   size_in_bytes
        ,   flags
        );
        
        if (MGBASE_UNLIKELY(!found_tag))
            return false;
        
        fjmpi_error::assert_zero(
            FJMPI_Rdma_get(src_proc, tag, raddr, laddr, size_in_bytes, flags)
        );
        
        set_notification(src_proc, nic, tag, on_complete);
        
        return true;
    }
    
    bool poll_nic(const int nic, int* const pid, int* const tag) MGBASE_NOEXCEPT
    {
        FJMPI_Rdma_cq cq;
        const int ret = FJMPI_Rdma_poll_cq(nic, &cq);
        
        if (ret == FJMPI_RDMA_NOTICE) {
            notify(nic, cq.pid, cq.tag);
            
            *pid = cq.pid;
            *tag = cq.tag;
            
            return true;
        }
        else
            return false;
    }
    
private:
    bool try_new_tag(const int proc, const int nic, int* const tag_result) MGBASE_NOEXCEPT
    {
        MGBASE_LOG_DEBUG("msg:Try to get a new tag.\tproc:{}\tnic:{}", proc, nic);
        std::queue<int>& q = info_by_procs_[proc].by_nics[nic].free_tags;
        //if (!info_by_procs_[proc].by_nics[nic].free_tags.try_dequeue(tag))
        if (q.empty())
            return false;
        
        MGBASE_LOG_DEBUG("msg:Got a new tag.\tsize:{}", q.size());
        
        const int tag = q.front();
        q.pop();
        
        *tag_result = tag;
        
        return true;
    }
    
    void set_notification(const int proc, const int nic, const int tag, const mgbase::operation& on_complete) MGBASE_NOEXCEPT
    {
        info_by_procs_[proc].by_nics[nic].by_tags[tag].on_complete = on_complete;
    }
    
    void notify(const int nic, const int pid, const int tag) MGBASE_NOEXCEPT
    {
        nic_info& info = info_by_procs_[pid].by_nics[nic];
        mgbase::execute(info.by_tags[tag].on_complete);
        
        MGBASE_LOG_DEBUG("msg:Recycle tag.\tsize:{}", info.free_tags.size());
        info.free_tags.push(tag);
    }
    
    struct tag_info {
        mgbase::operation on_complete;
    };
    
    struct nic_info {
        tag_info by_tags[max_tag_count];
        
        // Use a 32-bit integer because only 32-bit or 64-bit atomic operations are supported on SPARC
        //mgbase::static_bounded_index_queue<mgbase::int32_t, 16> free_tags;
        std::queue<int> free_tags;
        
        nic_info() {
            for (mgbase::int32_t tag = 0; tag < max_tag_count; ++tag)
                free_tags.push(tag);
                //free_tags.enqueue(tag);
        }
    };
    
    struct processor_info {
        nic_info by_nics[max_nic_count];
    };
    
    processor_info* info_by_procs_;
    mgbase::mpmc_bounded_queue<int> memid_queue_;
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

