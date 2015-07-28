
#include <mpi.h>
//#include <mpi-ext.h>
#include "mpi-ext.h"

#include <mgcom.hpp>

#include <algorithm>

#include <mgbase/lockfree/static_bounded_index_queue.hpp>
#include <mgbase/threading/spinlock.hpp>

#include "impl.hpp"

#include <mgbase/stdint.hpp>

namespace mgcom {

namespace {

class fjmpi_error { };

class com_fjmpi
{
    typedef mgbase::spinlock  lock_type;
    
public:
    void initialize(int* argc, char*** argv) {
        int provided;
        ::MPI_Init_thread(argc, argv, MPI_THREAD_SERIALIZED, &provided);
        
        int ret = ::FJMPI_Rdma_init();
        if (ret != 0)
            throw fjmpi_error();
        
        int size, rank;
        ::MPI_Comm_size(MPI_COMM_WORLD, &size);
        ::MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        
        process_id_ = static_cast<process_id_t>(rank);
        number_of_processes_ = static_cast<index_t>(size);
        
        info_by_procs_ = new processor_info[number_of_processes_];
        
        for (std::size_t i = 0; i < max_nic_count; i++)
            number_of_outstandings_[i].store(0, mgbase::memory_order_relaxed);
        
        next_nic_ = 0;
    }
    
    void finalize() {
        int ret = ::FJMPI_Rdma_finalize();
        if (ret != 0)
            throw fjmpi_error();
        
        ::MPI_Finalize();
        
        delete[] info_by_procs_;
    }
    
    int register_memory(void* buf, std::size_t length, mgbase::uint64_t* address_result) {
        int memid = new_memid();
        
        mgbase::uint64_t address = ::FJMPI_Rdma_reg_mem(memid, buf, length);
        if (address == FJMPI_RDMA_ERROR)
            throw fjmpi_error();
        
        *address_result = address;
        return memid;
    }
    
    void deregister_memory(int memid) {
        int ret = ::FJMPI_Rdma_dereg_mem(memid);
        if (ret != 0)
            throw fjmpi_error();
        
        free_memid(memid);
    }
    
    bool try_put_async(int dest, mgbase::uint64_t laddr, mgbase::uint64_t raddr, std::size_t size_in_bytes, const notifier_t& on_complete) {
        const int nic = select_nic(dest);
        
        int tag;
        if (!try_new_tag(dest, nic, &tag))
            return false;
        
        if (mpi_lock_.try_lock()) {
            const int ret = ::FJMPI_Rdma_put(dest, tag, raddr, laddr, size_in_bytes, nic);
            mpi_lock_.unlock();
            
            if (ret != 0)
                throw fjmpi_error();
            
            set_notifier(dest, nic, tag, on_complete);
            return true;
        }
        
        free_tag(dest, nic, tag);
        return false;
    }
    
    bool try_get_async(int dest, mgbase::uint64_t laddr, mgbase::uint64_t raddr, std::size_t size_in_bytes, const notifier_t& on_complete) {
        const int nic = select_nic(dest);
        
        int tag;
        if (!try_new_tag(dest, nic, &tag))
            return false;
        
        if (mpi_lock_.try_lock()) {
            const int ret = ::FJMPI_Rdma_get(dest, tag, raddr, laddr, size_in_bytes, nic);
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
    int new_memid();
    void free_memid(int memid);
    
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
    
    void set_notifier(int proc, int nic, int tag, const notifier_t& on_complete) MGBASE_NOEXCEPT {
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
        notifier_t on_complete;
    };
    
    struct nic_info {
        tag_info by_tags[max_tag_count];
        
        // Use a 32-bit integer because of the support of atomic operations on SPARC
        mgbase::static_bounded_index_queue<mgbase::int32_t, 16> free_tags;
    };
    
    struct processor_info {
        nic_info by_nics[max_nic_count];
        int prev_nic;
    };
    
    lock_type mpi_lock_;
    
    mgcom_process_id_t process_id_;
    mgcom_index_t number_of_processes_;
    
    int next_nic_;
    mgbase::atomic<mgbase::uint32_t> number_of_outstandings_[max_nic_count];
    processor_info* info_by_procs_;
    
};


com_fjmpi g_com;

}

void initialize(int* argc, char*** argv) {
    g_com.initialize(argc, argv);
}

void finalize() {
    g_com.finalize();
}

local_region_t register_region(
    void*                          local_pointer
,   index_t                        size_in_bytes
) {
    mgbase::uint64_t address;
    int memid = g_com.register_memory(local_pointer, size_in_bytes, &address);
    
    local_region_t region;
    region.local_id = static_cast<local_region_id_t>(memid);
    region.local_address = address;
    return region;
}

void deregister_region(
    local_region_t                 local_region
,   void*                          /*local_pointer*/
,   index_t                        /*size_in_bytes*/
) {
    g_com.deregister_memory(static_cast<int>(local_region.local_id));
}

bool try_write_async(
    local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
) {
    return g_com.try_put_async(
        static_cast<int>(dest_proc),
        get_absolute_address(local_address),
        get_absolute_address(remote_address),
        size_in_bytes,
        on_complete
    );
}

bool try_read_async(
    local_address_t                local_address
,   remote_address_t               remote_address
,   index_t                        size_in_bytes
,   process_id_t                   dest_proc
,   notifier_t                     on_complete
) {
    return g_com.try_get_async(
        static_cast<int>(dest_proc),
        get_absolute_address(local_address),
        get_absolute_address(remote_address),
        size_in_bytes,
        on_complete
    );
}

void poll() {
    g_com.poll();
}

void barrier() {
    ::MPI_Barrier(MPI_COMM_WORLD);
}

}

