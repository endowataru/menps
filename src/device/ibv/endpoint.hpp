
#pragma once

#include "connection.hpp"
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/logger.hpp>

namespace mgcom {

namespace ibv {

class endpoint
{
    static const mgbase::uint32_t num_cqe = 1024;
    
public:
    void initialize()
    {
        conns_          = new connection[mgcom::number_of_processes()];
        local_qp_nums_  = new mgbase::uint32_t[mgcom::number_of_processes()];
        remote_qp_nums_ = new mgbase::uint32_t[mgcom::number_of_processes()];
        lids_           = new mgbase::uint16_t[mgcom::number_of_processes()];
        
        init_ibv();
    }
    
    void finalize()
    {
        lids_.reset();
        remote_qp_nums_.reset();
        local_qp_nums_.reset();
        conns_.reset();
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
    
    ibv_mr* register_memory(
        void* const             buf
    ,   const mgbase::size_t    size_in_bytes
    ) {
        MGBASE_ASSERT(pd_ != MGBASE_NULLPTR);
        
        ibv_mr* const mr = ibv_reg_mr(pd_, buf, size_in_bytes, 
            IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
            IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC);
        
        if (mr == MGBASE_NULLPTR)
            throw ibv_error();
        
        MGBASE_LOG_DEBUG(
            "msg:Registered region.\t"
            "ptr:{:x}\tsize_in_bytes:{}\t"
            "lkey:{:x}\trkey:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(buf)
        ,   size_in_bytes
        ,   mr->lkey
        ,   mr->rkey
        );
        
        return mr;
    }
    
    void deregister_memory(ibv_mr* const mr)
    {
        ibv_dereg_mr(mr);
    }
    
    // TODO
    ibv_cq* get_cq() {
        return cq_;
    }
    
private:
    void init_ibv() {
        open_device();
        query_device();
        
        alloc_pd();
        create_cq();
        create_qp();
        
        query_port();
        exchange_qp_nums();
        exchange_lids();
        
        modify_qp();
    }
    
    void open_device()
    {
        int num_devices;
        ibv_device** const dev_list = ibv_get_device_list(&num_devices);
        
        if (dev_list == MGBASE_NULLPTR || num_devices < 0)
            throw ibv_error();
        
        // Use dev_list[0] (TODO)
        
        context_ = ibv_open_device(dev_list[0]);
        if (context_ == MGBASE_NULLPTR)
            throw ibv_error();
        
        ibv_free_device_list(dev_list);
    }
    
    void query_device()
    {
        const int ret = ibv_query_device(context_, &device_attr_);
        if (ret != 0)
            throw ibv_error();
    }
    
    void alloc_pd() {
        pd_ = ibv_alloc_pd(context_);
        if (pd_ == MGBASE_NULLPTR)
            throw ibv_error();
    }
    
    void create_cq() {
        cq_ = ibv_create_cq(context_, num_cqe, MGBASE_NULLPTR, MGBASE_NULLPTR, 0);
        if (cq_ == MGBASE_NULLPTR)
            throw ibv_error();
    }
    
    void create_qp() {
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].create(cq_, pd_);
    }
    
    void query_port() {
        // TODO: Port number is fixed to 1
        const int ret = ibv_query_port(context_, 1, &port_attr_);
        if (ret != 0)
            throw ibv_error();
    }
    
    void exchange_qp_nums()
    {
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            local_qp_nums_[proc] = conns_[proc].get_qp_num();
        
        mgcom::collective::alltoall(&local_qp_nums_[0], &remote_qp_nums_[0], 1);
        
        /*int ret = MPI_Alltoall(
            local_qp_nums_ , sizeof(mgbase::uint32_t), MPI_BYTE,
            remote_qp_nums_, sizeof(mgbase::uint32_t), MPI_BYTE,
            MPI_COMM_WORLD
        );
        if (ret != MPI_SUCCESS)
            throw ibv_error();*/
    }
    
    void exchange_lids()
    {
        mgcom::collective::allgather(&port_attr_.lid, &lids_[0], 1);
        
        /*int ret = MPI_Allgather(
            &port_attr_.lid, sizeof(mgbase::uint16_t), MPI_BYTE,
            lids_          , sizeof(mgbase::uint16_t), MPI_BYTE,
            MPI_COMM_WORLD
        );
        if (ret != MPI_SUCCESS)
            throw ibv_error();*/
    }
    
    void modify_qp() {
        for (process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
            conns_[proc].start(remote_qp_nums_[proc], lids_[proc], device_attr_);
    }
    
    ibv_context*    context_;
    ibv_pd*         pd_;
    ibv_cq*         cq_;
    mgbase::scoped_ptr<connection [] > conns_;
    
    ibv_device_attr device_attr_;
    ibv_port_attr   port_attr_;
    mgbase::scoped_ptr<mgbase::uint32_t []> local_qp_nums_;
    mgbase::scoped_ptr<mgbase::uint32_t []> remote_qp_nums_;
    mgbase::scoped_ptr<mgbase::uint16_t []> lids_;
};

} // namespace ibv

} // namespace mgcom

