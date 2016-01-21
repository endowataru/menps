
#pragma once

#include "rma.hpp"
#include "device/mpi/mpi_base.hpp"

#include "mpi-ext.h"

#include <algorithm>

#include <mgbase/lockfree/static_bounded_index_queue.hpp>
#include <mgbase/lockfree/mpmc_bounded_queue.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

#include "device/fjmpi/fjmpi_error.hpp"

#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

class impl
{
    typedef mgbase::spinlock  lock_type;
    
public:
    impl()
        : memid_queue_(max_tag_count + 1) { }
    
    void initialize()
    {
        int ret = ::FJMPI_Rdma_init();
        if (ret != 0)
            throw fjmpi_error();
        
        info_by_procs_ = new processor_info[number_of_processes()];
        
        for (std::size_t i = 0; i < max_nic_count; i++)
            number_of_outstandings_[i].store(0, mgbase::memory_order_relaxed);
        
        for (int memid = 0; memid < max_memid_count; memid++)
            memid_queue_.enqueue(memid);
        
        next_nic_ = 0;
    }
    
    void finalize() {
        int ret = ::FJMPI_Rdma_finalize();
        if (ret != 0)
            throw fjmpi_error();
        
        delete[] info_by_procs_;
    }
    
    int register_memory(void* buf, std::size_t length, mgbase::uint64_t* address_result)
    {
        int memid;
        if (!memid_queue_.dequeue(memid)) {
            MGBASE_LOG_WARN("Exceeded the capacity of memid");
            throw fjmpi_error(); // exceeded the capacity of memid
        }
        
        mgbase::uint64_t address;
        {
            // TODO: Blocking
            mgbase::lock_guard<lock_type> lc(get_lock());
            address = ::FJMPI_Rdma_reg_mem(memid, buf, length);
        }
        if (address == FJMPI_RDMA_ERROR)
            throw fjmpi_error();
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region.\tptr:{:x}\tsize:{}\tladdr:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   length
        ,   address
        );
        
        *address_result = address;
        return memid;
    }
    
    mgbase::uint64_t get_remote_addr(int pid, int memid)
    {
        mgbase::uint64_t raddr;
        {
            // TODO: Blocking
            mgbase::lock_guard<lock_type> lc(get_lock());
            raddr = ::FJMPI_Rdma_get_remote_addr(pid, memid);
        }
        if (raddr == FJMPI_RDMA_ERROR)
            throw fjmpi_error();
        
        MGBASE_LOG_DEBUG(
            "msg:Got remote address.\tproc:{}\tmemid:{}\traddr:{:x}"
        ,   pid
        ,   memid
        ,   raddr
        );
        
        return raddr;
    }
    
    void deregister_memory(int memid)
    {
        int ret;
        {
            // TODO: Blocking
            mgbase::lock_guard<lock_type> lc(get_lock());
            ret = ::FJMPI_Rdma_dereg_mem(memid);
        }
        if (ret != 0)
            throw fjmpi_error();
        
        memid_queue_.enqueue(memid);
    }
    
    bool try_put(
        int                         dest_proc
    ,   mgbase::uint64_t            laddr
    ,   mgbase::uint64_t            raddr
    ,   std::size_t                 size_in_bytes
    ,   const mgbase::operation&    on_complete
    ,   int                         extra_flags
    ) {
        const int nic = select_nic(dest_proc);
        
        int tag;
        if (!try_new_tag(dest_proc, nic, &tag))
            return false;
        
        if (get_lock().try_lock()) {
            const int flags = nic | extra_flags;
            
            MGBASE_LOG_DEBUG(
                "msg:RDMA Put.\tdest_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
            ,   dest_proc
            ,   laddr
            ,   raddr
            ,   size_in_bytes
            ,   flags
            );
            
            const int ret = ::FJMPI_Rdma_put(dest_proc, tag, raddr, laddr, size_in_bytes, flags);
            get_lock().unlock();
            
            if (ret != 0)
                throw fjmpi_error();
            
            set_operation(dest_proc, nic, tag, on_complete);
            return true;
        }
        
        free_tag(dest_proc, nic, tag);
        return false;
    }
    
    bool try_get(
        int                         src_proc
    ,   mgbase::uint64_t            laddr
    ,   mgbase::uint64_t            raddr
    ,   std::size_t                 size_in_bytes
    ,   const mgbase::operation&    on_complete
    ,   int                         extra_flags
    ) {
        const int nic = select_nic(src_proc);
        
        int tag;
        if (!try_new_tag(src_proc, nic, &tag))
            return false;
        
        if (get_lock().try_lock()) {
            const int flags = nic | extra_flags;
            
            MGBASE_LOG_DEBUG(
                "msg:RDMA Get.\tsrc_proc:{}\tladdr:{:x}\traddr:{:x}\tsize_in_bytes:{}\tflags:{}"
            ,   src_proc
            ,   laddr
            ,   raddr
            ,   size_in_bytes
            ,   flags
            );
            
            const int ret = ::FJMPI_Rdma_get(src_proc, tag, raddr, laddr, size_in_bytes, flags);
            get_lock().unlock();
            
            if (ret != 0)
                throw fjmpi_error();
            
            set_operation(src_proc, nic, tag, on_complete);
            return true;
        }
        
        free_tag(src_proc, nic, tag);
        return false;
    }
    
private:
    int select_nic(int proc) MGBASE_NOEXCEPT {
        return mod_by_nic_count(info_by_procs_[proc].prev_nic + 1);
    }
    
    bool try_new_tag(int proc, int nic, int* tag_result) MGBASE_NOEXCEPT {
        mgbase::int32_t tag;
        if (!info_by_procs_[proc].by_nics[nic].free_tags.try_dequeue(tag))
            return false;
        
        *tag_result = tag;
        
        return true;
    }
    
    void set_operation(int proc, int nic, int tag, const mgbase::operation& on_complete) MGBASE_NOEXCEPT
    {
        info_by_procs_[proc].by_nics[nic].by_tags[tag].on_complete = on_complete;
        number_of_outstandings_[nic].fetch_add(1, mgbase::memory_order_relaxed);
        
        next_nic_ = nic;
    }
    
    void free_tag(int proc, int nic, int tag) MGBASE_NOEXCEPT {
        info_by_procs_[proc].by_nics[nic].free_tags.enqueue(tag);
    }
    
public:
    bool poll() {
        const int next_nic = next_nic_;
        int nic = next_nic;
        do {
            const mgbase::uint32_t number_of_outstandings = number_of_outstandings_[nic].load(mgbase::memory_order_relaxed);
            if (number_of_outstandings > 0)
                return poll_nic(nic);
            
            nic = mod_by_nic_count(nic + 1);
        }
        while (nic != next_nic);
        
        return false;
    }
    
private:
    bool poll_nic(int nic) {
        if (!get_lock().try_lock())
            return false;
        
        ::FJMPI_Rdma_cq cq;
        int ret = ::FJMPI_Rdma_poll_cq(nic, &cq);
        
        get_lock().unlock();
        
        if (ret == FJMPI_RDMA_NOTICE)
            notify(nic, cq.pid, cq.tag);
        
        return true;
    }
    
private:
    static lock_type& get_lock() MGBASE_NOEXCEPT {
        return mpi_base::get_lock();
    }
    
    static const int max_memid_count = 510;
    static const int max_tag_count = 15;
    static const int max_nic_count = 4;
    
    static int mod_by_nic_count(int n) {
        return n & (max_nic_count - 1);
    }
    
    void notify(int nic, int pid, int tag) MGBASE_NOEXCEPT {
        nic_info& info = info_by_procs_[pid].by_nics[nic];
        mgbase::execute(info.by_tags[tag].on_complete);
        info.free_tags.enqueue(tag);
        number_of_outstandings_[nic].fetch_sub(1, mgbase::memory_order_relaxed);
    }
    
    struct tag_info {
        mgbase::operation on_complete;
    };
    
    struct nic_info {
        tag_info by_tags[max_tag_count];
        
        // Use a 32-bit integer because only 32-bit or 64-bit atomic operations are supported on SPARC
        mgbase::static_bounded_index_queue<mgbase::int32_t, 16> free_tags;
        
        nic_info() {
            for (mgbase::int32_t tag = 0; tag < max_tag_count; ++tag)
                free_tags.enqueue(tag);
        }
    };
    
    struct processor_info {
        nic_info by_nics[max_nic_count];
        int prev_nic;
    };
    
    int next_nic_;
    mgbase::atomic<mgbase::uint32_t> number_of_outstandings_[max_nic_count];
    processor_info* info_by_procs_;
    mgbase::mpmc_bounded_queue<int> memid_queue_;
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

