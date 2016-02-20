
#pragma once

#include "device/mpi/command/mpi_completer.hpp"
#include "device/fjmpi/fjmpi.hpp"
#include "device/fjmpi/fjmpi_error.hpp"
#include <mgbase/container/circular_buffer.hpp>

#include "device/fjmpi/remote_notice.hpp"

namespace mgcom {
namespace fjmpi {

class fjmpi_completer
{
public:
    static const int max_tag_count = 15; // Decided by FJMPI specification
    static const int max_nic_count = 4; // Decided by FJMPI specification
    
    void initialize()
    {
        mpi1_.initialize();
        
        info_by_procs_ = new processor_info[number_of_processes()];
    }
    
    void finalize()
    {
        info_by_procs_.reset();
        
        mpi1_.finalize();
    }
    
    mpi::mpi_completer& get_mpi1_completer() { return mpi1_; }
    
    bool try_new_tag(
        const int proc
    ,   const int local_nic
    ,   int* const tag_result
    ) MGBASE_NOEXCEPT
    {
        MGBASE_LOG_DEBUG("msg:Try to get a new tag.\tproc:{}\tnic:{}", proc, local_nic);
        
        tag_queue_type& q = info_by_procs_[proc].by_nics[local_nic].free_tags;
        if (MGBASE_UNLIKELY(q.empty()))
            return false;
        
        const int tag = q.front();
        q.pop_front();
        
        MGBASE_LOG_DEBUG("msg:Got a new tag.\ttag:{}\tqueue_size:{}", tag, q.size());
        
        *tag_result = tag;
        
        return true;
    }
    
    void set_notification(
        const int                   proc
    ,   const int                   nic
    ,   const int                   tag
    ,   const mgbase::operation&    on_complete
    ) MGBASE_NOEXCEPT
    {
        info_by_procs_[proc].by_nics[nic].by_tags[tag].on_complete = on_complete;
    }
    
    MGBASE_ALWAYS_INLINE
    void poll_on_this_thread()
    {
        // RDMA polling
        int pid;
        int tag;
        poll_nic(poll_nic_, &pid, &tag);
        
        if (MGBASE_UNLIKELY(++poll_nic_ >= fjmpi_completer::max_nic_count))
            poll_nic_ = 0;
        
        // MPI polling
        mpi1_.poll_on_this_thread();
    }
    
private:
    MGBASE_ALWAYS_INLINE
    bool poll_nic(const int nic, int* const pid, int* const tag) MGBASE_NOEXCEPT
    {
        FJMPI_Rdma_cq cq;
        const int ret = FJMPI_Rdma_poll_cq(nic, &cq);
        
        if (ret == FJMPI_RDMA_NOTICE) {
            MGBASE_LOG_DEBUG(
                "msg:Polled FJMPI_RDMA_NOTICE.\t"
                "nic:{}\tpid:{}\ttag:{}"
            ,   nic
            ,   cq.pid
            ,   cq.tag
            );
            
            notify(nic, cq.pid, cq.tag);
            
            *pid = cq.pid;
            *tag = cq.tag;
            
            return true;
        }
        else if (ret == FJMPI_RDMA_HALFWAY_NOTICE)
        {
            MGBASE_LOG_DEBUG(
                "msg:Polled FJMPI_RDMA_HALFWAY_NOTICE.\t"
                "nic:{}\tpid:{}\ttag:{}"
            ,   nic
            ,   cq.pid
            ,   cq.tag
            );
            
            mgcom::fjmpi::remote_notice(nic, cq.pid, cq.tag);
            
            return true;
        }
        else
            return false;
    }
    
    MGBASE_ALWAYS_INLINE
    void notify(const int nic, const int pid, const int tag) MGBASE_NOEXCEPT
    {
        nic_info& info = info_by_procs_[pid].by_nics[nic];
        mgbase::execute(info.by_tags[tag].on_complete);
        
        MGBASE_LOG_DEBUG("msg:Recycle tag.\tnic:{}\tpid:{}\ttag:{}\tqueue_size:{}", nic, pid, tag, info.free_tags.size());
        info.free_tags.push_back(tag);
    }
    
    struct tag_info {
        mgbase::operation on_complete;
    };
    
    typedef mgbase::static_circular_buffer<int, max_tag_count> tag_queue_type;
    
    struct nic_info {
        tag_info by_tags[max_tag_count];
        
        // Use a 32-bit integer because only 32-bit or 64-bit atomic operations are supported on SPARC
        tag_queue_type free_tags;
        
        nic_info() {
            for (mgbase::int32_t tag = 0; tag < max_tag_count; ++tag)
                free_tags.push_back(tag);
                //free_tags.enqueue(tag);
        }
    };
    
    struct processor_info {
        nic_info by_nics[max_nic_count];
    };
    
    mgbase::scoped_ptr<processor_info []> info_by_procs_;
    int poll_nic_;
    
    mpi::mpi_completer mpi1_;
};

} // namespace fjmpi
} // namespace mgcom

