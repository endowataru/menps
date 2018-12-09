
#pragma once

#include <menps/mecom/rma/command_queue.hpp>
#include "device/ibv/command/atomic_buffer.hpp"
#include "device/ibv/rma/address.hpp"
#include "device/ibv/command/tag_queue.hpp"

namespace menps {
namespace mecom {
namespace ibv {

inline void make_wr_to(
    const rma::async_untyped_read_params&   params
,   const wr_id_t                           wr_id
,   ibv_send_wr* const                      wr
,   ibv_sge* const                          sge
,   tag_queue&                              tag_que
,   atomic_buffer&                          //atomic_buf
) {
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_READ;
    wr->send_flags            = 0;
    wr->wr.rdma.remote_addr   = to_raddr(params.src_raddr);
    wr->wr.rdma.rkey          = to_rkey(params.src_raddr);
    
    sge->addr   = to_laddr(params.dest_laddr);
    sge->length = static_cast<mefdn::uint32_t>(params.size_in_bytes);
    sge->lkey   = to_lkey(params.dest_laddr);
    
    tag_que.set_on_complete(wr_id, params.on_complete);
}

inline void make_wr_to(
    const rma::async_untyped_write_params&  params
,   const wr_id_t                           wr_id
,   ibv_send_wr* const                      wr
,   ibv_sge* const                          sge
,   tag_queue&                              tag_que
,   atomic_buffer&                          //atomic_buf
) {
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_WRITE;
    
    wr->send_flags            =
        (params.size_in_bytes < MECOM_IBV_SEND_INLINE_SIZE) ? IBV_SEND_INLINE : 0;
    
    wr->wr.rdma.remote_addr   = to_raddr(params.dest_raddr);
    wr->wr.rdma.rkey          = to_rkey(params.dest_raddr);
    
    sge->addr   = to_laddr(params.src_laddr);
    sge->length = static_cast<mefdn::uint32_t>(params.size_in_bytes);
    sge->lkey   = to_lkey(params.src_laddr);
    
    tag_que.set_on_complete(wr_id, params.on_complete);
}

template <typename T>
inline void make_wr_to(
    const rma::async_atomic_read_params<T>& params
,   const wr_id_t                           wr_id
,   ibv_send_wr* const                      wr
,   ibv_sge* const                          sge
,   tag_queue&                              tag_que
,   atomic_buffer&                          atomic_buf
) {
    const auto r =
        atomic_buf.make_notification_read(wr_id, params.on_complete, params.dest_ptr);
    
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_READ;
    wr->send_flags            = 0; // TODO
    wr->wr.rdma.remote_addr   = to_raddr(params.src_rptr);
    wr->wr.rdma.rkey          = to_rkey(params.src_rptr);
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(T);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

template <typename T>
inline void make_wr_to(
    const rma::async_atomic_write_params<T>&    params
,   const wr_id_t                               wr_id
,   ibv_send_wr* const                          wr
,   ibv_sge* const                              sge
,   tag_queue&                                  tag_que
,   atomic_buffer&                              atomic_buf
) {
    const auto r = atomic_buf.make_notification_write(wr_id, params.on_complete);
    
    // Assign a value to the buffer.
    *r.buf_lptr = params.value;
    
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_RDMA_WRITE;
    wr->send_flags            =
        (sizeof(T) < MECOM_IBV_SEND_INLINE_SIZE) ? IBV_SEND_INLINE : 0;
    
    wr->wr.rdma.remote_addr   = to_raddr(params.dest_rptr);
    wr->wr.rdma.rkey          = to_rkey(params.dest_rptr);
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(T);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

template <typename T>
inline void make_wr_to(
    const rma::async_compare_and_swap_params<T>&    params
,   const wr_id_t                                   wr_id
,   ibv_send_wr* const                              wr
,   ibv_sge* const                                  sge
,   tag_queue&                                      tag_que
,   atomic_buffer&                                  atomic_buf
) {
    const auto r = atomic_buf.make_notification_atomic(wr_id, params.on_complete, params.result_ptr);
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_ATOMIC_CMP_AND_SWP;
    wr->send_flags            = 0; // TODO
    wr->wr.atomic.remote_addr = to_raddr(params.target_rptr);
    wr->wr.atomic.compare_add = params.expected;
    wr->wr.atomic.swap        = params.desired;
    wr->wr.atomic.rkey        = to_rkey(params.target_rptr);
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(T);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}

template <typename T>
inline void make_wr_to(
    const rma::async_fetch_and_add_params<T>&   params
,   const wr_id_t                               wr_id
,   ibv_send_wr* const                          wr
,   ibv_sge* const                              sge
,   tag_queue&                                  tag_que
,   atomic_buffer&                              atomic_buf
) {
    const auto r = atomic_buf.make_notification_atomic(wr_id, params.on_complete, params.result_ptr);
    const auto buf_laddr = r.buf_lptr.to_address();
    
    wr->wr_id                 = wr_id;
    // wr->next is unset
    wr->sg_list               = sge;
    wr->num_sge               = 1;
    wr->opcode                = IBV_WR_ATOMIC_FETCH_AND_ADD;
    wr->send_flags            = 0; // TODO
    wr->wr.atomic.remote_addr = to_raddr(params.target_rptr);
    wr->wr.atomic.compare_add = params.value;
    // wr->wr.atomic.swap is unused
    wr->wr.atomic.rkey        = to_rkey(params.target_rptr);
    
    sge->addr   = to_laddr(buf_laddr);
    sge->length = sizeof(T);
    sge->lkey   = to_lkey(buf_laddr);
    
    tag_que.set_on_complete(wr_id, r.on_complete);
}


inline void set_command_to(
    const rma::command&     cmd
,   const mefdn::uint64_t  wr_id
,   ibv_send_wr* const      wr
,   ibv_sge* const          sge
,   tag_queue&              comp
,   atomic_buffer&          atomic_buf
)
{
    switch (cmd.code)
    {
        // TODO: safer & cleaner code...
        
        case rma::command_code::read:
            make_wr_to(reinterpret_cast<const rma::async_untyped_read_params&>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case rma::command_code::write:
            make_wr_to(reinterpret_cast<const rma::async_untyped_write_params&>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case rma::command_code::atomic_read:
            make_wr_to(reinterpret_cast<const rma::async_atomic_read_params<rma::atomic_default_t>&>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case rma::command_code::atomic_write:
            make_wr_to(reinterpret_cast<const rma::async_atomic_write_params<rma::atomic_default_t>&>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case rma::command_code::compare_and_swap:
            make_wr_to(reinterpret_cast<const rma::async_compare_and_swap_params<rma::atomic_default_t>&>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        case rma::command_code::fetch_and_add:
            make_wr_to(reinterpret_cast<const rma::async_fetch_and_add_params<rma::atomic_default_t>&>(cmd.arg), wr_id, wr, sge, comp, atomic_buf);
            break;
        
        MEFDN_COVERED_SWITCH()
    }
}

} // namespace ibv
} // namespace mecom
} // namespace menps
