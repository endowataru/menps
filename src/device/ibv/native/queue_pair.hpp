
#pragma once

#include "verbs.hpp"
#include <mgbase/assert.hpp>

#include "device/ibv/ibv_error.hpp"

#include <limits>

#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

class queue_pair
    : mgbase::noncopyable
{
    static const mgbase::uint32_t max_send_wr = 1024;
    static const mgbase::uint32_t max_recv_wr = 1024;
    static const mgbase::uint32_t max_send_sge = 1; // Scatter/Gather
    static const mgbase::uint32_t max_recv_sge = 1;
    
public:
    queue_pair()
        : qp_(MGBASE_NULLPTR) { }
    
    ~queue_pair() {
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
        
        attr.qp_type             = IBV_QPT_RC; // Reliable queue_pair (RC)
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
        attr.qp_type             = IBV_QPT_RC; // Reliable queue_pair (RC)
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
    MGBASE_WARN_UNUSED_RESULT
    bool try_post_send(ibv_send_wr& wr) const
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        ibv_send_wr* bad_wr;
        const int err = ibv_post_send(qp_, &wr, &bad_wr);
        
        if (MGBASE_LIKELY(err == 0)) {
            log_wr("Posted IBV request.", wr);
            return true;
        }
        else if (err == ENOMEM) {
            log_wr("Used up the memory for posting IBV request.", wr);
            return false;
        }
        else {
            log_wr("Failed to post an IBV request.", wr);
            throw ibv_error("ibv_post_send() failed", err);
        }
    }
    
private:
    void log_wr(const char* const msg, const ibv_send_wr& wr) const
    {
        switch (wr.opcode)
        {
            case IBV_WR_RDMA_WRITE: {
                MGBASE_LOG_DEBUG(
                    "msg:{}\topcode:{}\t"
                    "wr_id:{}\traddr:{:x}\trkey:{:x}\t"
                    "laddr:{:x}\tlkey:{:x}\tsize_in_bytes:{}"
                ,   msg
                ,   "IBV_WR_RDMA_WRITE"
                ,   wr.wr_id
                ,   wr.wr.rdma.remote_addr
                ,   wr.wr.rdma.rkey
                ,   wr.sg_list[0].addr // TODO: show as a list
                ,   wr.sg_list[0].lkey
                ,   wr.sg_list[0].length
                );
                break;
            }
            case IBV_WR_RDMA_READ: {
                MGBASE_LOG_DEBUG(
                    "msg:{}\topcode:{}\t"
                    "wr_id:{}\traddr:{:x}\trkey:{:x}\t"
                    "laddr:{:x}\tlkey:{:x}\tsize_in_bytes:{}"
                ,   msg
                ,   "IBV_WR_RDMA_READ"
                ,   wr.wr_id
                ,   wr.wr.rdma.remote_addr
                ,   wr.wr.rdma.rkey
                ,   wr.sg_list[0].addr // TODO: show as a list
                ,   wr.sg_list[0].lkey
                ,   wr.sg_list[0].length
                );
                break;
            }
            case IBV_WR_ATOMIC_CMP_AND_SWP: {
                MGBASE_LOG_DEBUG(
                    "msg:{}\topcode:{}\t"
                    "wr_id:{}\traddr:{:x}\trkey:{:x}\t"
                    "laddr:{:x}\tlkey:{:x}\tlength:{}\t"
                    "expected:{}\tdesired:{}"
                ,   msg
                ,   "IBV_WR_ATOMIC_CMP_AND_SWP"
                ,   wr.wr_id
                ,   wr.wr.atomic.remote_addr
                ,   wr.wr.atomic.rkey
                ,   wr.sg_list[0].addr // TODO: show as a list
                ,   wr.sg_list[0].lkey
                ,   wr.sg_list[0].length
                ,   wr.wr.atomic.compare_add
                ,   wr.wr.atomic.swap
                );
                break;
            }
            case IBV_WR_ATOMIC_FETCH_AND_ADD: {
                MGBASE_LOG_DEBUG(
                    "msg:{}\topcode:{}\t"
                    "wr_id:{}\traddr:{:x}\trkey:{:x}\t"
                    "laddr:{:x}\tlkey:{:x}\tlength:{}\t"
                    "value:{}"
                ,   msg
                ,   "IBV_WR_ATOMIC_FETCH_AND_ADD"
                ,   wr.wr_id
                ,   wr.wr.atomic.remote_addr
                ,   wr.wr.atomic.rkey
                ,   wr.sg_list[0].addr // TODO: show as a list
                ,   wr.sg_list[0].lkey
                ,   wr.sg_list[0].length
                ,   wr.wr.atomic.compare_add
                );
                break;
            }
            default: {
                MGBASE_LOG_DEBUG(
                    "msg:{}\t"
                    "wr_id:{}\topcode:{} (unknown)"
                ,   msg
                ,   wr.wr_id
                ,   wr.opcode
                );
                break;
            }
        }
    }

private:
    ibv_qp* qp_;
};

} // namespace ibv
} // namespace mgcom

