
#pragma once

#include "params.hpp"
#include "atomic_buffer.hpp"
#include "device/ibv/rma/address.hpp"
#include "tag_queue.hpp"

namespace mgcom {
namespace ibv {

inline void make_wr_to(
    const write_command&    cmd
,   const mgbase::uint64_t  wr_id
,   ibv_send_wr* const      wr
,   ibv_sge* const          sge
,   tag_queue&              tag_que
,   atomic_buffer&          //atomic_buf
) {
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_WRITE;
    
    wr->send_flags            =
        (cmd.size_in_bytes < MGCOM_IBV_SEND_INLINE_SIZE) ? IBV_SEND_INLINE : 0;
    
    wr->wr.rdma.remote_addr   = cmd.raddr;
    wr->wr.rdma.rkey          = cmd.rkey;
    
    sge->addr   = cmd.laddr;
    sge->length = static_cast<mgbase::uint32_t>(cmd.size_in_bytes);
    sge->lkey   = cmd.lkey;
    
    tag_que.set_on_complete(wr_id, cmd.on_complete);
}

inline void make_wr_to(
    const read_command&     cmd
,   const mgbase::uint64_t  wr_id
,   ibv_send_wr* const      wr
,   ibv_sge* const          sge
,   tag_queue&              tag_que
,   atomic_buffer&          //atomic_buf
) {
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_READ;
    wr->send_flags            = 0; // TODO
    wr->wr.rdma.remote_addr   = cmd.raddr;
    wr->wr.rdma.rkey          = cmd.rkey;
    
    sge->addr   = cmd.laddr;
    sge->length = static_cast<mgbase::uint32_t>(cmd.size_in_bytes);
    sge->lkey   = cmd.lkey;
    
    tag_que.set_on_complete(wr_id, cmd.on_complete);
}

inline void make_wr_to(
    const atomic_read_command&  cmd
,   const mgbase::uint64_t      wr_id
,   ibv_send_wr* const          wr
,   ibv_sge* const              sge
,   tag_queue&                  tag_que
,   atomic_buffer&              atomic_buf
) {
    const auto r = atomic_buf.make_notification_read(wr_id, cmd.on_complete, cmd.dest_ptr);
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_READ;
    wr->send_flags            = 0; // TODO
    wr->wr.rdma.remote_addr   = cmd.raddr;
    wr->wr.rdma.rkey          = cmd.rkey;
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(mgbase::uint64_t);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

inline void make_wr_to(
    const atomic_write_command& cmd
,   const mgbase::uint64_t      wr_id
,   ibv_send_wr* const          wr
,   ibv_sge* const              sge
,   tag_queue&                  tag_que
,   atomic_buffer&              atomic_buf
) {
    const auto r = atomic_buf.make_notification_write(wr_id, cmd.on_complete);
    
    // Assign a value to the buffer.
    *r.buf_lptr = cmd.value;
    
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_WRITE;
    wr->send_flags            =
        (sizeof(mgbase::uint64_t) < MGCOM_IBV_SEND_INLINE_SIZE) ? IBV_SEND_INLINE : 0;
    
    wr->wr.rdma.remote_addr   = cmd.raddr;
    wr->wr.rdma.rkey          = cmd.rkey;
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(mgbase::uint64_t);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

inline void make_wr_to(
    const compare_and_swap_command& cmd
,   const mgbase::uint64_t          wr_id
,   ibv_send_wr* const              wr
,   ibv_sge* const                  sge
,   tag_queue&                      tag_que
,   atomic_buffer&                  atomic_buf
) {
    const auto r = atomic_buf.make_notification_atomic(wr_id, cmd.on_complete, cmd.result_ptr);
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_ATOMIC_CMP_AND_SWP;
    wr->send_flags            = 0; // TODO
    wr->wr.atomic.remote_addr = cmd.raddr;
    wr->wr.atomic.compare_add = cmd.expected;
    wr->wr.atomic.swap        = cmd.desired;
    wr->wr.atomic.rkey        = cmd.rkey;
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(mgbase::uint64_t);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

inline void make_wr_to(
    const fetch_and_add_command&    cmd
,   const mgbase::uint64_t          wr_id
,   ibv_send_wr* const              wr
,   ibv_sge* const                  sge
,   tag_queue&                      tag_que
,   atomic_buffer&                  atomic_buf
) {
    const auto r = atomic_buf.make_notification_atomic(wr_id, cmd.on_complete, cmd.result_ptr);
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_ATOMIC_FETCH_AND_ADD;
    wr->send_flags            = 0; // TODO
    wr->wr.atomic.remote_addr = cmd.raddr;
    wr->wr.atomic.compare_add = cmd.value;
    // wr->wr.atomic.swap is unused
    wr->wr.atomic.rkey        = cmd.rkey;
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(mgbase::uint64_t);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

} // namespace ibv
} // namespace mgcom

