
#pragma once

#include "params.hpp"

namespace mgcom {
namespace ibv {

inline void make_wr_to(
    const write_params& params
,   ibv_send_wr* const  wr
,   ibv_sge* const      sge
) {
    wr->wr_id                 = params.wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_WRITE;
    wr->send_flags            = 0; // TODO
    wr->wr.rdma.remote_addr   = params.raddr;
    wr->wr.rdma.rkey          = params.rkey;
    
    sge->addr   = params.laddr;
    sge->length = static_cast<mgbase::uint32_t>(params.size_in_bytes);
    sge->lkey   = params.lkey;
}

inline void make_wr_to(
    const read_params&  params
,   ibv_send_wr* const  wr
,   ibv_sge* const      sge
) {
    wr->wr_id                 = params.wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_READ;
    wr->send_flags            = 0; // TODO
    wr->wr.rdma.remote_addr   = params.raddr;
    wr->wr.rdma.rkey          = params.rkey;
    
    sge->addr   = params.laddr;
    sge->length = static_cast<mgbase::uint32_t>(params.size_in_bytes);
    sge->lkey   = params.lkey;
}

inline void make_wr_to(
    const compare_and_swap_params&  params
,   ibv_send_wr* const              wr
,   ibv_sge* const                  sge
) {
    wr->wr_id                 = params.wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_ATOMIC_CMP_AND_SWP;
    wr->send_flags            = 0; // TODO
    wr->wr.atomic.remote_addr = params.raddr;
    wr->wr.atomic.compare_add = params.expected;
    wr->wr.atomic.swap        = params.desired;
    wr->wr.atomic.rkey        = params.rkey;
    
    sge->addr   = params.laddr;
    sge->length = sizeof(mgbase::uint64_t);
    sge->lkey   = params.lkey;
}

inline void make_wr_to(
    const fetch_and_add_params& params
,   ibv_send_wr* const          wr
,   ibv_sge* const              sge
) {
    wr->wr_id                 = params.wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_ATOMIC_FETCH_AND_ADD;
    wr->send_flags            = 0; // TODO
    wr->wr.atomic.remote_addr = params.raddr;
    wr->wr.atomic.compare_add = params.value;
    // wr->wr.atomic.swap is unused
    wr->wr.atomic.rkey        = params.rkey;
    
    sge->addr   = params.laddr;
    sge->length = sizeof(mgbase::uint64_t);
    sge->lkey   = params.lkey;
}

} // namespace ibv
} // namespace mgcom

