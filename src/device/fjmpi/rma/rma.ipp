
#pragma once

#include "rma.hpp"
#include "common/mpi_base.hpp"

#include "mpi-ext.h"

#include <algorithm>

#include <mgbase/lockfree/static_bounded_index_queue.hpp>
#include <mgbase/lockfree/mpmc_bounded_queue.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

#include "fjmpi_error.hpp"

namespace mgcom {
namespace rma {

namespace {

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
        if (!memid_queue_.dequeue(memid))
            throw fjmpi_error(); // exceeded the capacity of memid
        
        mgbase::uint64_t address;
        {
            // TODO: Blocking
            mgbase::lock_guard<lock_type> lc(mpi_lock_);
            address = ::FJMPI_Rdma_reg_mem(memid, buf, length);
        }
        if (address == FJMPI_RDMA_ERROR)
            throw fjmpi_error();
        
        *address_result = address;
        return memid;
    }
    
    mgbase::uint64_t get_remote_addr(int pid, int memid)
    {
        mgbase::uint64_t raddr;
        {
            // TODO: Blocking
            mgbase::lock_guard<lock_type> lc(mpi_lock_);
            raddr = ::FJMPI_Rdma_get_remote_addr(pid, memid);
        }
        if (raddr == FJMPI_RDMA_ERROR)
            throw fjmpi_error();
        
        return raddr;
    }
    
    void deregister_memory(int memid)
    {
        int ret;
        {
            // TODO: Blocking
            mgbase::lock_guard<lock_type> lc(mpi_lock_);
            ret = ::FJMPI_Rdma_dereg_mem(memid);
        }
        if (ret != 0)
            throw fjmpi_error();
        
        memid_queue_.enqueue(memid);
    }
    
    bool try_put(int dest, mgbase::uint64_t laddr, mgbase::uint64_t raddr, std::size_t size_in_bytes, const local_notifier& on_complete, int extra_flags)
    {
        const int nic = select_nic(dest);
        
        int tag;
        if (!try_new_tag(dest, nic, &tag))
            return false;
        
        if (mpi_lock_.try_lock()) {
            const int flags = nic | extra_flags;
            
            const int ret = ::FJMPI_Rdma_put(dest, tag, raddr, laddr, size_in_bytes, flags);
            mpi_lock_.unlock();
            
            if (ret != 0)
                throw fjmpi_error();
            
            set_notifier(dest, nic, tag, on_complete);
            return true;
        }
        
        free_tag(dest, nic, tag);
        return false;
    }
    
    bool try_get(int dest, mgbase::uint64_t laddr, mgbase::uint64_t raddr, std::size_t size_in_bytes, const local_notifier& on_complete, int extra_flags) {
        const int nic = select_nic(dest);
        
        int tag;
        if (!try_new_tag(dest, nic, &tag))
            return false;
        
        if (mpi_lock_.try_lock()) {
            const int flags = nic | extra_flags;
            
            const int ret = ::FJMPI_Rdma_get(dest, tag, raddr, laddr, size_in_bytes, flags);
            mpi_lock_.unlock();
            
            if (ret != 0)
                throw fjmpi_error();
            
            set_notifier(dest, nic, tag, on_complete);
            return true;
        }
        
        free_tag(dest, nic, tag);
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
    
    void set_notifier(int proc, int nic, int tag, const local_notifier& on_complete) MGBASE_NOEXCEPT {
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
        if (!mpi_lock_.try_lock())
            return false;
        
        ::FJMPI_Rdma_cq cq;
        int ret = ::FJMPI_Rdma_poll_cq(nic, &cq);
        
        mpi_lock_.unlock();
        
        if (ret == FJMPI_RDMA_NOTICE)
            notify(nic, cq.pid, cq.tag);
        
        return true;
    }
    
private:
    static const int max_memid_count = 510;
    static const int max_tag_count = 15;
    static const int max_nic_count = 4;
    
    static int mod_by_nic_count(int n) {
        return n & (max_nic_count - 1);
    }
    
    void notify(int nic, int pid, int tag) MGBASE_NOEXCEPT {
        nic_info& info = info_by_procs_[pid].by_nics[nic];
        mgcom::notify(info.by_tags[tag].on_complete);
        info.free_tags.enqueue(tag);
        number_of_outstandings_[nic].fetch_sub(1, mgbase::memory_order_relaxed);
    }
    
    struct tag_info {
        local_notifier on_complete;
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
    
    lock_type mpi_lock_;
    
    int next_nic_;
    mgbase::atomic<mgbase::uint32_t> number_of_outstandings_[max_nic_count];
    processor_info* info_by_procs_;
    mgbase::mpmc_bounded_queue<int> memid_queue_;
};

}

}
}

