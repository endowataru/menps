
#include <mgdev/ibv/queue_pair.hpp>
#include <mgbase/logger.hpp>
#include <limits>

namespace mgdev {
namespace ibv {

void queue_pair_deleter::operator () (ibv_qp* const qp) const MGBASE_NOEXCEPT
{
    if (qp == MGBASE_NULLPTR) {
        // Ignore deletion of nullptr.
        return;
    }
    
    ibv_destroy_qp(qp); // Ignore error
}

qp_init_attr_t make_default_rc_qp_init_attr()
{
    static const mgbase::uint32_t default_max_send_wr = 1 << 14; // TODO: magic number
    static const mgbase::uint32_t default_max_recv_wr = 1 << 14; // TODO: magic number
    static const mgbase::uint32_t default_max_send_sge = 1; // Scatter/Gather
    static const mgbase::uint32_t default_max_recv_sge = 1;
    
    qp_init_attr_t attr = qp_init_attr_t();
        // Notice: zero filled by value initialization
    
    // Both for ibv_qp_init_attr & ibv_exp_qp_init_attr
    attr.qp_context          = MGBASE_NULLPTR;
    attr.qp_type             = IBV_QPT_RC; // Reliable queue_pair (RC)
    attr.send_cq             = MGBASE_NULLPTR; // Filled by users
    attr.recv_cq             = MGBASE_NULLPTR; // Filled by users
    attr.srq                 = MGBASE_NULLPTR;
    attr.cap.max_send_wr     = default_max_send_wr;
    attr.cap.max_recv_wr     = default_max_recv_wr;
    attr.cap.max_send_sge    = default_max_send_sge;
    attr.cap.max_recv_sge    = default_max_recv_sge;
    attr.cap.max_inline_data = 1; // TODO
    attr.sq_sig_all          = 1;
    
    #ifdef MGDEV_IBV_EXP_SUPPORTED
    // Only for ibv_exp_qp_init_attr
    // See also: ctx_exp_qp_create() of perftest
    
    attr.pd                  = MGBASE_NULLPTR; // Set by make_queue_pair later
    
    // Only use attr.pd by default.
    attr.comp_mask = IBV_EXP_QP_INIT_ATTR_PD;
    #endif
    
    return attr;
}
qp_init_attr_t make_default_rc_qp_init_attr(const device_attr_t& dev_attr)
{
    auto attr = make_default_rc_qp_init_attr();
    
    #ifdef MGDEV_IBV_MASKED_ATOMICS_SUPPORTED
    if (is_only_masked_atomics(dev_attr))
    {
        // Enable attr.exp_create_flags too.
        attr.comp_mask |= IBV_EXP_QP_INIT_ATTR_CREATE_FLAGS;
        
        attr.max_atomic_arg     = 8; // TODO : correct?
        attr.exp_create_flags   |= IBV_EXP_QP_CREATE_ATOMIC_BE_REPLY;
        attr.comp_mask          |= IBV_EXP_QP_INIT_ATTR_ATOMICS_ARG;
    }
    #endif
    
    return attr;
}

queue_pair make_queue_pair(
    ibv_pd* const           pd
,   qp_init_attr_t* const   attr
) {
    #ifdef MGDEV_IBV_EXP_SUPPORTED
    attr->pd = pd;
    const auto ctx = pd->context;
    
    const auto qp = ibv_exp_create_qp(ctx, attr);
    if (qp == MGBASE_NULLPTR)
        throw ibv_error("ibv_exp_create_qp() failed");
    
    #else
    const auto qp = ibv_create_qp(pd, attr);
    if (qp == MGBASE_NULLPTR)
        throw ibv_error("ibv_create_qp() failed");
    #endif
    
    return queue_pair(qp);
}


qp_attr_t make_default_qp_attr()
{
    qp_attr_t attr = qp_attr_t();
        // Notice: zero filled by value initialization
    
    //qp_attr.qp_state        = IBV_QPS_INIT;
    attr.pkey_index      = 0;
    attr.port_num        = 1; // 1 or 2
    attr.qp_access_flags =
        IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
            // Allow all operations
    
    //attr.qp_state              = IBV_QPS_RTR;
    attr.path_mtu              = IBV_MTU_4096;
    attr.dest_qp_num           = 0; // Filled by users
    attr.rq_psn                = 0; // PSN starts from 0
    attr.max_dest_rd_atomic    = 0; // Filled by users
    attr.max_rd_atomic         = 0; // Filled by users
    attr.min_rnr_timer         = 0; // Arbitrary from 0 to 31 (TODO: Is it true?)
    attr.ah_attr.is_global     = 0; // Doesn't use Global Routing Header (GRH)
    attr.ah_attr.dlid          = 0; // Filled by users
    attr.ah_attr.sl            = 0;
    attr.ah_attr.src_path_bits = 0;
    attr.ah_attr.port_num      = 1; // 1 or 2
    attr.ah_attr.static_rate   = 0;
    
    //attr.qp_state      = IBV_QPS_RTS;
    attr.timeout       = 0; // Arbitrary from 0 to 31 (TODO: Is it true?)
    attr.retry_cnt     = 7; // Arbitrary from 0 to 7
    attr.rnr_retry     = 7; // TODO
    attr.sq_psn        = 0; // Arbitrary
    //attr.max_rd_atomic = 16; // TODO : Usually 0 ?
    
    return attr;
}
qp_attr_t make_default_qp_attr(const device_attr_t& dev_attr)
{
    auto qp_attr = make_default_qp_attr();
    
    // Assume that both of the sender/receiver are using the same HCA product.
    qp_attr.max_rd_atomic =
        static_cast<mgbase::uint8_t>(
            std::min(
                dev_attr.max_qp_rd_atom
            ,   static_cast<int>(std::numeric_limits<mgbase::uint8_t>::max())
            )
        );
    
    qp_attr.max_dest_rd_atomic =
        static_cast<mgbase::uint8_t>(
            std::min(
                dev_attr.max_qp_init_rd_atom
            ,   static_cast<int>(std::numeric_limits<mgbase::uint8_t>::max())
            )
        );
    
    return qp_attr;
}

void set_qp_dest(qp_attr_t* const attr, const global_qp_id dest_qp_id)
{
    attr->ah_attr.dlid      = dest_qp_id.node_id.lid;
    attr->ah_attr.port_num  = dest_qp_id.port_num;
    
    attr->dest_qp_num       = dest_qp_id.qp_num;
}


#ifdef MGDEV_IBV_EXP_SUPPORTED
    #define MODIFY_QP   ibv_exp_modify_qp
#else
    #define MODIFY_QP   ibv_modify_qp
#endif

void modify_reset_to_init(ibv_qp* const qp, qp_attr_t* const attr)
{
    attr->qp_state = IBV_QPS_INIT;
    
    // Reset -> Init
    const int ret =
        MODIFY_QP(
            qp
        ,   attr
        ,   IBV_QP_STATE | IBV_QP_PKEY_INDEX |
            IBV_QP_PORT | IBV_QP_ACCESS_FLAGS
        );
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (Reset to Init)");
}

void modify_init_to_rtr(ibv_qp* const qp, qp_attr_t* const attr)
{
    attr->qp_state = IBV_QPS_RTR;
    
    // Init -> RTR
    const int ret =
        MODIFY_QP(
            qp
        ,   attr
        ,   IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
            IBV_QP_DEST_QPN | IBV_QP_RQ_PSN |
            IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER
        );
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (Init to RTR)");
}

void modify_rtr_to_rts(ibv_qp* const qp, qp_attr_t* const attr)
{
    attr->qp_state = IBV_QPS_RTS;
    
    // RTR -> RTS
    const int ret =
        MODIFY_QP(
            qp
        ,   attr
        ,   IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
            IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC
        );
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (RTR to RTS)");
}

#undef MODIFY_QP



#if 0


#ifdef MGDEV_IBV_EXP_SUPPORTED
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
    
    #ifdef MGDEV_IBV_MASKED_ATOMICS_SUPPORTED
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

#ifdef MGDEV_IBV_MASKED_ATOMICS_SUPPORTED
    #define QP_ATTR     ibv_exp_qp_attr
    #define MODIFY_QP   ibv_exp_modify_qp
#else
    #define QP_ATTR     ibv_qp_attr
    #define MODIFY_QP   ibv_modify_qp
#endif

inline void modify_qp_reset_to_init(ibv_qp* const qp)
{
    QP_ATTR qp_attr = QP_ATTR();
    
    qp_attr.qp_state        = IBV_QPS_INIT;
    qp_attr.pkey_index      = 0;
    qp_attr.port_num        = 1; // 1 or 2
    qp_attr.qp_access_flags =
        IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
            // Allow all operations
    
    // Reset -> Init
    const int ret = MODIFY_QP(qp, &qp_attr,
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
    
    QP_ATTR attr = QP_ATTR();
    attr.qp_state              = IBV_QPS_RTR;
    attr.path_mtu              = IBV_MTU_4096;
    attr.dest_qp_num           = qp_num;
    attr.rq_psn                = 0; // PSN starts from 0
    attr.max_dest_rd_atomic    = static_cast<mgbase::uint8_t>(max_dest_rd_atomic);
    attr.max_rd_atomic         = 16;
    attr.min_rnr_timer         = 0; // Arbitrary from 0 to 31 (TODO: Is it true?)
    attr.ah_attr.is_global     = 0; // Doesn't use Global Routing Header (GRH)
    attr.ah_attr.dlid          = lid;
    attr.ah_attr.sl            = 0;
    attr.ah_attr.src_path_bits = 0;
    attr.ah_attr.port_num      = 1; // 1 or 2
    attr.ah_attr.static_rate   = 0;
    
    // Init -> RTR
    const int ret = MODIFY_QP(qp, &attr,
        IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
        IBV_QP_DEST_QPN | IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER);
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (Init to RTR)");
}

inline void modify_qp_rtr_to_rts(ibv_qp* const qp)
{
    QP_ATTR attr = QP_ATTR();
    attr.qp_state      = IBV_QPS_RTS;
    attr.timeout       = 0; // Arbitrary from 0 to 31 (TODO: Is it true?)
    attr.retry_cnt     = 7; // Arbitrary from 0 to 7
    attr.rnr_retry     = 7; // TODO
    attr.sq_psn        = 0; // Arbitrary
    attr.max_rd_atomic = 16; // TODO : Usually 0 ?
    
    // RTR to RTS
    const int ret = MODIFY_QP(qp, &attr,
        IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
        IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);
    
    if (ret != 0)
        throw ibv_error("ibv_modify_qp() failed (RTR to RTS)");
}

#undef QP_ATTR
#undef MODIFY_QP

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

#endif

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
        case IBV_WR_SEND_WITH_IMM: {
            MGBASE_LOG_DEBUG(
                "msg:{}\t"
                "wr_id:{}\topcode:{} (unknown)"
            ,   msg
            ,   wr.wr_id
            ,   wr.opcode
            );
            break;
        }
        
        MGBASE_COVERED_SWITCH()
    }
}

} // namespace ibv
} // namespace mgdev

