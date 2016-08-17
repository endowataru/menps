
#include "queue_pair.hpp"

#include <limits>

#include <mgbase/logger.hpp>

namespace mgcom {
namespace ibv {

queue_pair::queue_pair()
    : qp_(MGBASE_NULLPTR) { }

namespace /*unnamed*/ {

void destroy(ibv_qp* const qp)
{
    MGBASE_ASSERT(qp != MGBASE_NULLPTR);
    
    ibv_destroy_qp(qp); // Ignore error
}

} // unnamed namespace

queue_pair::~queue_pair() {
    if (qp_ != MGBASE_NULLPTR) {
        destroy(qp_);
    }
}

#ifdef MGCOM_IBV_EXP_SUPPORTED
void queue_pair::create(ibv_context& ctx, ibv_cq& cq, ibv_pd& pd)
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
void queue_pair::create(ibv_context& /*ctx*/, ibv_cq& cq, ibv_pd& pd)
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

namespace /*unnamed*/ {

inline void modify_qp_reset_to_init(ibv_qp* const qp)
{
    ibv_qp_attr qp_attr = ibv_qp_attr();
    qp_attr.qp_state        = IBV_QPS_INIT;
    qp_attr.pkey_index      = 0;
    qp_attr.port_num        = 1; // 1 or 2
    qp_attr.qp_access_flags =
        IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
            // Allow all operations
    
    // Reset -> Init
    const int ret = ibv_modify_qp(qp, &qp_attr,
        IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (Reset -> Init)");
}

inline void modify_qp_init_to_rtr(
    ibv_qp* const           qp
,   const mgbase::uint32_t  qp_num
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
    const int ret = ibv_modify_qp(qp, &attr,
        IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
        IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (Init to RTR)");
}

inline void modify_qp_rtr_to_rts(ibv_qp* const qp)
{
    ibv_qp_attr attr = ibv_qp_attr();
    attr.qp_state      = IBV_QPS_RTS;
    attr.timeout       = 0; // Arbitary from 0 to 31 (TODO: Is it true?)
    attr.retry_cnt     = 7; // Arbitary from 0 to 7
    attr.rnr_retry     = 7; // TODO
    attr.sq_psn        = 0; // Arbitary
    attr.max_rd_atomic = 16; // TODO : Usually 0 ?
    
    // RTR to RTS
    const int ret = ibv_modify_qp(qp, &attr,
        IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
        IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (RTR to RTS)");
}

} // unnamed namespace

void queue_pair::start(
    const mgbase::uint32_t  qp_num
,   const mgbase::uint16_t  lid
,   const ibv_device_attr&  device_attr
) {
    MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
    
    modify_qp_reset_to_init(qp_);
    modify_qp_init_to_rtr(qp_, qp_num, lid, device_attr);
    modify_qp_rtr_to_rts(qp_);
}

void queue_pair::log_wr_impl(const char* const msg, const ibv_send_wr& wr) const
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
        case IBV_WR_RDMA_WRITE_WITH_IMM:
        case IBV_WR_SEND:
        case IBV_WR_SEND_WITH_IMM:
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

} // namespace ibv
} // namespace mgcom

