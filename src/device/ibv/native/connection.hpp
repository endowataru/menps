
#pragma once

#include "verbs.hpp"
#include <mgbase/assert.hpp>

#include "device/ibv/ibv_error.hpp"

#include <limits>
#include <algorithm>

namespace mgcom {
namespace ibv {

class connection
    : mgbase::noncopyable
{
    static const mgbase::uint32_t max_send_wr = 1024;
    static const mgbase::uint32_t max_recv_wr = 1024;
    static const mgbase::uint32_t max_send_sge = 1; // Scatter/Gather
    static const mgbase::uint32_t max_recv_sge = 1;
    
public:
    connection()
        : qp_(MGBASE_NULLPTR) { }
    
    ~connection() {
        if (qp_ != MGBASE_NULLPTR)
            destroy();
    }
    
    mgbase::uint32_t get_qp_num() const MGBASE_NOEXCEPT {
        return qp_->qp_num;
    }
    
    #ifdef MGCOM_IBV_EXP_SUPPORTED
    void create(ibv_context& ctx, ibv_cq& cq, ibv_pd& pd)
    {
        // See also: ctx_exp_qp_create() of perftest
        
        ibv_exp_qp_init_attr attr = ibv_exp_qp_init_attr();
            // Notice: zero filled by value initialization
        
        attr.comp_mask = IBV_EXP_QP_INIT_ATTR_PD | IBV_EXP_QP_INIT_ATTR_CREATE_FLAGS;
            // Both flags are not documented
        
        attr.qp_type             = IBV_QPT_RC; // Reliable Connection (RC)
        attr.pd                  = &pd;
        attr.send_cq             = &cq;
        attr.recv_cq             = &cq;
        attr.srq                 = MGBASE_NULLPTR;
        attr.cap.max_send_wr     = max_send_wr;
        attr.cap.max_recv_wr     = max_recv_wr;
        attr.cap.max_send_sge    = max_send_sge;
        attr.cap.max_recv_sge    = max_recv_sge;
        attr.cap.max_inline_data = 1; // TODO
        attr.sq_sig_all          = 1;
        
        #ifdef MGCOM_IBV_MASKED_ATOMICS_SUPPORTED
        attr.max_atomic_arg      = 8; // TODO : correct?
        attr.exp_create_flags |= IBV_EXP_QP_CREATE_ATOMIC_BE_REPLY;
        #endif
        
        qp_ = ibv_exp_create_qp(&ctx, &attr);
        if (qp_ == MGBASE_NULLPTR)
            throw ibv_error("ibv_exp_create_qp() failed");
    }
    #else
    void create(ibv_context& /*ctx*/, ibv_cq& cq, ibv_pd& pd)
    {
        MGBASE_ASSERT(qp_ == MGBASE_NULLPTR);
        
        ibv_qp_init_attr attr = ibv_qp_init_attr();
            // Notice: zero filled by value initialization
        
        attr.qp_context          = MGBASE_NULLPTR;
        attr.qp_type             = IBV_QPT_RC; // Reliable Connection (RC)
        attr.send_cq             = &cq;
        attr.recv_cq             = &cq;
        attr.srq                 = MGBASE_NULLPTR;
        attr.cap.max_send_wr     = max_send_wr;
        attr.cap.max_recv_wr     = max_recv_wr;
        attr.cap.max_send_sge    = max_send_sge;
        attr.cap.max_recv_sge    = max_recv_sge;
        attr.cap.max_inline_data = 1; // TODO
        attr.sq_sig_all          = 1;
        
        qp_ = ibv_create_qp(&pd, &attr);
        if (qp_ == MGBASE_NULLPTR)
            throw ibv_error("ibv_create_qp() failed");
    }
    #endif
    
    void destroy()
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        ibv_destroy_qp(qp_); // Ignore error
        qp_ = MGBASE_NULLPTR;
    }
    
    void start(
        const mgbase::uint32_t  qp_num
    ,   const mgbase::uint16_t  lid
    ,   const ibv_device_attr&  device_attr
    ) {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        modify_qp_reset_to_init();
        modify_qp_init_to_rtr(qp_num, lid, device_attr);
        modify_qp_rtr_to_rts();
    }
    
private:
    void modify_qp_reset_to_init()
    {
        ibv_qp_attr qp_attr = ibv_qp_attr();
        qp_attr.qp_state        = IBV_QPS_INIT;
        qp_attr.pkey_index      = 0;
        qp_attr.port_num        = 1; // 1 or 2
        qp_attr.qp_access_flags =
            IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
                // Allow all operations
        
        // Reset -> Init
        const int ret = ibv_modify_qp(qp_, &qp_attr,
            IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);
        
        if (ret != 0)
            throw ibv_error("ibv_modify_qp() failed (Reset -> Init)");
    }
    
    void modify_qp_init_to_rtr(
        const mgbase::uint32_t  qp_num
    ,   const mgbase::uint16_t  lid
    ,   const ibv_device_attr&  device_attr
    ) {
        const int max_dest_rd_atomic = std::min(
            device_attr.max_qp_rd_atom
        ,   static_cast<int>(std::numeric_limits<mgbase::uint8_t>::max())
        );
        
        ibv_qp_attr attr = ibv_qp_attr();
        attr.qp_state              = IBV_QPS_RTR;
        attr.path_mtu              = IBV_MTU_4096;
        attr.dest_qp_num           = qp_num;
        attr.rq_psn                = 0; // PSN starts from 0
        attr.max_dest_rd_atomic    = static_cast<mgbase::uint8_t>(max_dest_rd_atomic);
        attr.max_rd_atomic         = 16;
        attr.min_rnr_timer         = 0; // Arbitary from 0 to 31 (TODO: Is it true?)
        attr.ah_attr.is_global     = 0; // Doesn't use Global Routing Header (GRH)
        attr.ah_attr.dlid          = lid;
        attr.ah_attr.sl            = 0;
        attr.ah_attr.src_path_bits = 0;
        attr.ah_attr.port_num      = 1; // 1 or 2
        attr.ah_attr.static_rate   = 0;
        
        // Init -> RTR
        const int ret = ibv_modify_qp(qp_, &attr,
            IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
            IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);
        
        if (ret != 0)
            throw ibv_error("ibv_modify_qp() failed (Init to RTR)");
    }
    
    void modify_qp_rtr_to_rts()
    {
        ibv_qp_attr attr = ibv_qp_attr();
        attr.qp_state      = IBV_QPS_RTS;
        attr.timeout       = 0; // Arbitary from 0 to 31 (TODO: Is it true?)
        attr.retry_cnt     = 7; // Arbitary from 0 to 7
        attr.rnr_retry     = 7; // TODO
        attr.sq_psn        = 0; // Arbitary
        attr.max_rd_atomic = 16; // TODO : Usually 0 ?
        
        // RTR to RTS
        const int ret = ibv_modify_qp(qp_, &attr,
            IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
            IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);
        
        if (ret != 0)
            throw ibv_error("ibv_modify_qp() failed (RTR to RTS)");
    }
    
public:
    bool try_write_async(
        const mgbase::uint64_t  wr_id
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const std::size_t       size_in_bytes
    ) const
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        ibv_sge sge = ibv_sge();
        sge.addr   = laddr;
        sge.length = static_cast<mgbase::uint32_t>(size_in_bytes);
        sge.lkey   = lkey;
        
        ibv_send_wr wr = ibv_send_wr();
        wr.wr_id               = wr_id;
        wr.next                = MGBASE_NULLPTR;
        wr.sg_list             = &sge;
        wr.num_sge             = 1;
        wr.opcode              = IBV_WR_RDMA_WRITE;
        wr.send_flags          = 0; // TODO
        wr.wr.rdma.remote_addr = raddr;
        wr.wr.rdma.rkey        = rkey;
        
        return post_send(wr);
    }
    
    bool try_read_async(
        const mgbase::uint64_t  wr_id
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const std::size_t       size_in_bytes
    ) const
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        ibv_sge sge = ibv_sge();
        sge.addr   = laddr;
        sge.length = static_cast<mgbase::uint32_t>(size_in_bytes);
        sge.lkey   = lkey;
        
        ibv_send_wr wr = ibv_send_wr();
        wr.wr_id               = wr_id;
        wr.next                = MGBASE_NULLPTR;
        wr.sg_list             = &sge;
        wr.num_sge             = 1;
        wr.opcode              = IBV_WR_RDMA_READ;
        wr.send_flags          = 0; // TODO
        wr.wr.rdma.remote_addr = raddr;
        wr.wr.rdma.rkey        = rkey;
        
        return post_send(wr);
    }
    
    bool try_compare_and_swap_async(
        const mgbase::uint64_t  wr_id
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const mgbase::uint64_t  expected
    ,   const mgbase::uint64_t  desired
    ) const
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        ibv_sge sge = ibv_sge();
        sge.addr   = laddr;
        sge.length = sizeof(mgbase::uint64_t);
        sge.lkey   = lkey;
        
        ibv_send_wr wr = ibv_send_wr();
        wr.wr_id                 = wr_id;
        wr.next                  = MGBASE_NULLPTR;
        wr.sg_list               = &sge;
        wr.num_sge               = 1;
        wr.opcode                = IBV_WR_ATOMIC_CMP_AND_SWP;
        wr.send_flags            = 0; // TODO
        wr.wr.atomic.remote_addr = raddr;
        wr.wr.atomic.compare_add = expected;
        wr.wr.atomic.swap        = desired;
        wr.wr.atomic.rkey        = rkey;
        
        return post_send(wr);
    }
    
    bool try_fetch_and_add_async(
        const mgbase::uint64_t  wr_id
    ,   const mgbase::uint64_t  raddr
    ,   const mgbase::uint32_t  rkey
    ,   const mgbase::uint64_t  laddr
    ,   const mgbase::uint32_t  lkey
    ,   const mgbase::uint64_t  value
    ) const
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        ibv_sge sge = ibv_sge();
        sge.addr   = laddr;
        sge.length = sizeof(mgbase::uint64_t);
        sge.lkey   = lkey;
        
        ibv_send_wr wr = ibv_send_wr();
        wr.wr_id                 = wr_id;
        wr.next                  = MGBASE_NULLPTR;
        wr.sg_list               = &sge;
        wr.num_sge               = 1;
        wr.opcode                = IBV_WR_ATOMIC_FETCH_AND_ADD;
        wr.send_flags            = 0; // TODO
        wr.wr.atomic.remote_addr = raddr;
        wr.wr.atomic.compare_add = value;
        // wr.wr.atomic.swap is unused
        wr.wr.atomic.rkey        = rkey;
        
        return post_send(wr);
    }
    
private:
    bool post_send(ibv_send_wr& wr) const
    {
        ibv_send_wr* bad_wr;
        const int err = ibv_post_send(qp_, &wr, &bad_wr);
        
        if (MGBASE_LIKELY(err == 0))
            return true;
        else if (err == ENOMEM)
            return false;
        else
            throw ibv_error("ibv_post_send() failed", err);
    }

private:
    ibv_qp* qp_;
};

} // namespace ibv
} // namespace mgcom

