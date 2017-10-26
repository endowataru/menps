
#pragma once

#include "meucp.hpp"

// synchronous functions =
//  application threads wait for returning from the function
// asynchronous functions = 
//  application threads do not wait for returning from the function

#define MEUCP_WORKER_FUNCS_SYNC_STATUS_WITHOUT_EP_CREATE() \
    X1(worker_query, \
        ucs_status_t, \
        ucp_worker_attr_t*, attr) \
    X2(worker_get_address,\
        ucs_status_t, \
        ucp_address_t**, address_p, \
        size_t*, address_length_p) \
    X1(worker_get_efd, \
        ucs_status_t, \
        int*, fd) \
    X0(worker_wait, \
        ucs_status_t) \
    X0(worker_arm, \
        ucs_status_t) \
    X0(worker_signal, \
        ucs_status_t) \
    X0(worker_fence, \
        ucs_status_t) \
    X0(worker_flush, \
        ucs_status_t) \
    X6(tag_recv_nbr, \
        ucs_status_t, \
        void *, buffer, \
        size_t, count, \
        ucp_datatype_t, datatype, \
        ucp_tag_t, tag, \
        ucp_tag_t, tag_mask, \
        void *, req) \

#define MEUCP_WORKER_FUNCS_SYNC_STATUS() \
    MEUCP_WORKER_FUNCS_SYNC_STATUS_WITHOUT_EP_CREATE() \
    X2(ep_create, \
        ucs_status_t, \
        const ucp_ep_params_t*, params, \
        ucp_ep_h*, ep)

#define MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR() \
    /* returns UCS_PTR_IS_ERR() or ucp_request* */ \
    X6(tag_recv_nb, \
        ucs_status_ptr_t, \
        void*, buffer, \
        size_t, count,  \
        ucp_datatype_t, datatype, \
        ucp_tag_t, tag, \
        ucp_tag_t, tag_mask, \
        ucp_tag_recv_callback_t, cb) \
    /* returns UCS_PTR_IS_ERR() or ucp_request* */ \
    X5(tag_msg_recv_nb, \
        ucs_status_ptr_t, \
        void*, buffer, \
        size_t, count, \
        ucp_datatype_t, datatype, \
        ucp_tag_message_h, message, \
        ucp_tag_recv_callback_t, cb)

#define MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR() \
    /* returns nullptr or ucp_recv_desc* (=ucp_tag_message_h) */ \
    X4(tag_probe_nb, \
        ucp_tag_message_h, \
        ucp_tag_t, tag, \
        ucp_tag_t, tag_mask, \
        int, remove, \
        ucp_tag_recv_info_t *, info)

#define MEUCP_WORKER_FUNCS_SYNC_VOID() \
    X1(worker_print_info, \
        void, \
        FILE*, stream) \
    X1(worker_release_address, \
        void, \
        ucp_address_t*, address) \
    X0(worker_progress, \
        void) \
    X1(worker_wait_mem, \
        void, \
        void*, address)

#define MEUCP_ENDPOINT_FUNCS_SYNC_STATUS() \
    X0(ep_flush, \
        ucs_status_t) \
    X4(put, \
        ucs_status_t, \
        const void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X4(get, \
        ucs_status_t, \
        void *, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \


/*  X2(ep_rkey_unpack, \
        ucs_status_t, \
        void*, rkey_buffer, \
        ucp_rkey_h*, rkey_p) \
*/

#define MEUCP_ENDPOINT_FUNCS_SYNC_VOID_WITHOUT_EP_DESTROY() \
    X1(ep_print_info, \
        void, \
        FILE*, stream)

#define MEUCP_ENDPOINT_FUNCS_SYNC_VOID() \
    MEUCP_ENDPOINT_FUNCS_SYNC_VOID_WITHOUT_EP_DESTROY() \
    X0(ep_destroy, \
        void)


#define MEUCP_ENDPOINT_FUNCS_SYNC_PTR_WITHOUT_DISCONNECT() \
    /* returns UCS_OK or UCS_PTR_IS_ERR() or ucp_request* */ \
    X7(atomic_fetch_nb, \
        ucs_status_ptr_t, \
        ucp_atomic_fetch_op_t, opcode, \
        uint64_t, value, \
        void*, result, \
        size_t, op_size, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        ucp_send_callback_t, cb) \
    /* returns UCS_OK or UCS_PTR_IS_ERR() or ucp_request* */ \
    X5(tag_send_nb, \
        ucs_status_ptr_t, \
        const void *, buffer, \
        size_t, count, \
        ucp_datatype_t, datatype, \
        ucp_tag_t, tag, \
        ucp_send_callback_t, cb) \
    /* returns UCS_PTR_IS_ERR() or ucp_request* */ \
    X5(tag_send_sync_nb, \
        ucs_status_ptr_t, \
        const void*, buffer, \
        size_t, count, \
        ucp_datatype_t, datatype, \
        ucp_tag_t, tag, \
        ucp_send_callback_t, cb) \

#define MEUCP_ENDPOINT_FUNCS_SYNC_PTR() \
    MEUCP_ENDPOINT_FUNCS_SYNC_PTR_WITHOUT_DISCONNECT() \
    X0(disconnect_nb, \
        ucs_status_ptr_t) 

#define MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS() \
    X4(put_nbi, \
        ucs_status_t, \
        const void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X4(get_nbi, \
        ucs_status_t, \
        void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X3(atomic_add32, \
        ucs_status_t, \
        uint32_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X3(atomic_add64, \
        ucs_status_t, \
        uint64_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X4(atomic_fadd32, \
        ucs_status_t, \
        uint32_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint32_t *, result) \
    X4(atomic_fadd64, \
        ucs_status_t, \
        uint64_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint64_t*, result)  \
    X4(atomic_swap32, \
        ucs_status_t, \
        uint32_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint32_t *, result) \
    X4(atomic_swap64, \
        ucs_status_t, \
        uint64_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint64_t*, result) \
    X5(atomic_cswap32, \
        ucs_status_t, \
        uint32_t, compare, \
        uint32_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint32_t*, result) \
    X5(atomic_cswap64, \
        ucs_status_t , \
        uint64_t, compare, \
        uint64_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint64_t *, result) \
    X5(atomic_post, \
        ucs_status_t, \
        ucp_atomic_post_op_t, opcode, \
        uint64_t, value, \
        size_t, op_size, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey)


#define MEUCP_REQUEST_FUNCS_SYNC_STATUS() \
    X2(request_test, \
        ucs_status_t, \
        void *, request, \
        ucp_tag_recv_info_t *, info)

// remaining:

//;




#define C ,

#define X0(name, tr) \
    DEF_BASE(name, tr, )
#define X1(name, tr, t0, a0) \
    DEF_BASE(name, tr, C t0 a0)
#define X2(name, tr, t0, a0, t1, a1) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1)
#define X3(name, tr, t0, a0, t1, a1, t2, a2) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2)
#define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3)
#define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3 C t4 a4)
#define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3 C t4 a4 C t5 a5)
#define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3 C t4 a4 C t5 a5 C t6 a6)

#define DEF_BASE(name, tr, args) \
    extern "C" tr MEUCP_REAL(ucp_ ## name)(ucp_worker_h wk args); \
    extern "C" tr MEUCP_WRAP(ucp_ ## name)(ucp_worker_h wk args);

MEUCP_WORKER_FUNCS_SYNC_STATUS()
MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()
MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
MEUCP_WORKER_FUNCS_SYNC_VOID()

#undef DEF_BASE

#define DEF_BASE(name, tr, args) \
    extern "C" tr MEUCP_REAL(ucp_ ## name)(ucp_ep_h wk args); \
    extern "C" tr MEUCP_WRAP(ucp_ ## name)(ucp_ep_h wk args);

MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
MEUCP_ENDPOINT_FUNCS_SYNC_VOID()
MEUCP_ENDPOINT_FUNCS_SYNC_PTR()
MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS()

#undef DEF_BASE

#define DEF_BASE(name, tr, args) \
    extern "C" tr MEUCP_REAL(ucp_ ## name)(void* req args); \
    extern "C" tr MEUCP_WRAP(ucp_ ## name)(void* req args);

MEUCP_REQUEST_FUNCS_SYNC_STATUS()

#undef DEF_BASE

#undef C
#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
#undef X5
#undef X6
#undef X7

