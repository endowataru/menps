
#pragma once

#include "connection.hpp"
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

class alltoall_connections
{
public:
    alltoall_connections() MGBASE_EMPTY_DEFINITION
    
    void create(ibv_cq& cq, ibv_pd& pd)
    {
        conns_ = new connection[mgcom::number_of_processes()];
        
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].create(cq, pd);
    }
    
    void destroy()
    {
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].destroy();
        
        conns_.reset();
    }
    
    void collective_start(const ibv_device_attr& device_attr, const ibv_port_attr& port_attr)
    {
        mgbase::scoped_ptr<mgbase::uint32_t []> local_qp_nums(
            new mgbase::uint32_t[mgcom::number_of_processes()]
        );
        mgbase::scoped_ptr<mgbase::uint32_t []> remote_qp_nums(
            new mgbase::uint32_t[mgcom::number_of_processes()]
        );
        mgbase::scoped_ptr<mgbase::uint16_t []> lids(
            new mgbase::uint16_t[mgcom::number_of_processes()]
        );
        
        // Extract qp_num.
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            local_qp_nums[proc] = conns_[proc].get_qp_num();
        
        // Exchange qp_num with all of the processes.
        mgcom::collective::alltoall(&local_qp_nums[0], &remote_qp_nums[0], 1);
        
        // Exchange lid with all of the processes.
        mgcom::collective::allgather(&port_attr.lid, &lids[0], 1);
        
        // Start all of the connections.
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].start(remote_qp_nums[proc], lids[proc], device_attr);
    }
    
    MGBASE_WARN_UNUSED_RESULT bool try_write_async(
        const mgbase::uint64_t  wr_id
    ,   const process_id_t      dest_proc
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const std::size_t       size_in_bytes
    ) {
        const bool ret = conns_[dest_proc].try_write_async(
            wr_id
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   size_in_bytes
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "wr_id:{}\tdest_proc:{}\traddr:{:x}\trkey:{:x}\t"
            "laddr:{:x}\tlkey:{:x}\tsize_in_bytes:{}"
        ,   (ret ? "Executed RDMA WRITE." : "Failed to execute RDMA WRITE.")
        ,   wr_id
        ,   dest_proc
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   size_in_bytes
        );
        
        return ret;
    }
    
    MGBASE_WARN_UNUSED_RESULT bool try_read_async(
        const mgbase::uint64_t  wr_id
    ,   const process_id_t      src_proc
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const std::size_t       size_in_bytes
    ) {
        const bool ret = conns_[src_proc].try_read_async(
            wr_id
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   size_in_bytes
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "wr_id:{}\tsrc_proc:{}\traddr:{:x}\trkey:{:x}\t"
            "laddr:{:x}\tlkey:{:x}\tsize_in_bytes:{}"
        ,   (ret ? "Executed RDMA READ." : "Failed to execute RDMA READ.")
        ,   wr_id
        ,   src_proc
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   size_in_bytes
        );
        
        return ret;
    }
    
    MGBASE_WARN_UNUSED_RESULT bool try_compare_and_swap_async(
        const mgbase::uint64_t  wr_id
    ,   const process_id_t      target_proc
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const mgbase::uint64_t  expected
    ,   const mgbase::uint64_t  desired
    ) {
        const bool ret = conns_[target_proc].try_compare_and_swap_async(
            wr_id
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   expected
        ,   desired
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "wr_id:{}\ttarget_proc:{}\traddr:{:x}\trkey:{:x}\t"
            "laddr:{:x}\tlkey:{:x}\texpected:{}\tdesired:{}"
        ,   (ret ? "Executed RDMA CAS." : "Failed to execute RDMA CAS.")
        ,   wr_id
        ,   target_proc
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   expected
        ,   desired
        );
        
        return ret;
    }
    
    MGBASE_WARN_UNUSED_RESULT bool try_fetch_and_add_async(
        const mgbase::uint64_t  wr_id
    ,   const process_id_t      target_proc
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const mgbase::uint64_t  value
    ) {
        const bool ret = conns_[target_proc].try_fetch_and_add_async(
            wr_id
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   value
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "wr_id:{}\ttarget_proc:{}\traddr:{:x}\trkey:{:x}\t"
            "laddr:{:x}\tlkey:{:x}\texpected:{}\tdesired:{}"
        ,   (ret ? "Executed RDMA FAA." : "Failed to execute RDMA FAA.")
        ,   wr_id
        ,   target_proc
        ,   raddr
        ,   rkey
        ,   laddr
        ,   lkey
        ,   value
        );
        
        return ret;
    }

private:
    mgbase::scoped_ptr<connection []> conns_;
};

} // namespace ibv
} // namespace mgcom

