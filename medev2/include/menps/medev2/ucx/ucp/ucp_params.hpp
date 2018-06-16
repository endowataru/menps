
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>

// void ucp_???(ucp_worker_h, ???);
#define MEDEV2_UCP_WORKER_FUNCS_SYNC_VOID(X) \
    X(worker_destroy, \
        void, \
        1, \
        ucp_worker_h, worker) \
    X(worker_release_address, \
        void, \
        2, \
        ucp_worker_h, worker, \
        ucp_address_t*, address)

// ucs_status_t ucp_???(ucp_worker_h, ???);
#define MEDEV2_UCP_WORKER_FUNCS_SYNC_STATUS(X) \
    X(worker_get_address, \
        ucs_status_t, \
        3, \
        ucp_worker_h, worker, \
        ucp_address_t**, address_p, \
        size_t*, address_length_p) \
    X(worker_fence, \
        ucs_status_t, \
        1, \
        ucp_worker_h, worker) \
    X(worker_query, \
        ucs_status_t, \
        2, \
        ucp_worker_h, worker, \
        ucp_worker_attr_t*, attr)

#define MEDEV2_UCP_WORKER_FUNCS_EP_CREATE(X) \
    X(ep_create, \
        ucs_status_t, \
        3, \
        ucp_worker_h, worker, \
        const ucp_ep_params_t*, params, \
        ucp_ep_h *, ep_p)

#define MEDEV2_UCP_WORKER_FUNCS_WORKER_PROGRESS(X) \
    X(worker_progress, \
        unsigned, \
        1, \
        ucp_worker_h, worker)


// ucs_status_t ucp_???(ucp_ep_h, ???);
#define MEDEV2_UCP_EP_FUNCS_SYNC_STATUS(X) \
    X(ep_rkey_unpack, \
        ucs_status_t, \
        3, \
        ucp_ep_h, ep, \
        const void*, rkey_buffer, \
        ucp_rkey_h*, rkey_p) \
    X(put, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        const void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X(atomic_add32, \
        ucs_status_t, \
        4, \
        ucp_ep_h, ep, \
        uint32_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X(atomic_add64, \
        ucs_status_t, \
        4, \
        ucp_ep_h, ep, \
        uint64_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X(atomic_fadd32, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        uint32_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint32_t *, result) \
    X(atomic_fadd64, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        uint64_t, add, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint64_t*, result)  \
    X(atomic_swap32, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        uint32_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint32_t *, result) \
    X(atomic_swap64, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        uint64_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint64_t*, result) \
    X(atomic_cswap32, \
        ucs_status_t, \
        6, \
        ucp_ep_h, ep, \
        uint32_t, compare, \
        uint32_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint32_t*, result) \
    X(atomic_cswap64, \
        ucs_status_t , \
        6, \
        ucp_ep_h, ep, \
        uint64_t, compare, \
        uint64_t, swap, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        uint64_t*, result) \

// ucs_status_t ucp_???(ucp_ep_h, ???);
#define MEDEV2_UCP_EP_FUNCS_ASYNC_STATUS(X) \
    X(put_nbi, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        const void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X(get_nbi, \
        ucs_status_t, \
        5, \
        ucp_ep_h, ep, \
        void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey) \
    X(atomic_post, \
        ucs_status_t, \
        6, \
        ucp_ep_h, ep, \
        ucp_atomic_post_op_t, opcode, \
        uint64_t, value, \
        size_t, op_size, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey)

// ucs_status_ptr_t ucp_???(ucp_ep_h, ???);
#define MEDEV2_UCP_EP_FUNCS_ASYNC_STATUS_PTR(X) \
    X(ep_flush_nb, \
        ucs_status_ptr_t, \
        3, \
        ucp_ep_h, ep, \
        unsigned, flags, \
        ucp_send_callback_t, cb) \
    X(stream_send_nb, \
        ucs_status_ptr_t, \
        6, \
        ucp_ep_h, ep, \
        const void*, buffer, \
        size_t, count, \
        ucp_datatype_t, datatype, \
        ucp_send_callback_t, cb, \
        unsigned, flags) \
    X(tag_send_nb, \
        ucs_status_ptr_t, \
        6, \
        ucp_ep_h, ep, \
        const void*, buffer, \
        size_t, count, \
        ucp_datatype_t, datatype, \
        ucp_tag_t, tag, \
        ucp_send_callback_t, cb) \
    X(put_nb, \
        ucs_status_ptr_t, \
        6, \
        ucp_ep_h, ep, \
        const void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        ucp_send_callback_t, cb) \
    X(get_nb, \
        ucs_status_ptr_t, \
        6, \
        ucp_ep_h, ep, \
        void*, buffer, \
        size_t, length, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        ucp_send_callback_t, cb) \
    X(atomic_fetch_nb, \
        ucs_status_ptr_t, \
        8, \
        ucp_ep_h, ep, \
        ucp_atomic_fetch_op_t, opcode, \
        uint64_t, value, \
        void*, result, \
        size_t, op_size, \
        uint64_t, remote_addr, \
        ucp_rkey_h, rkey, \
        ucp_send_callback_t, cb) \


#define MEDEV2_UCP_EP_FUNCS_EP_CLOSE_NB(X) \
    X(ep_close_nb, \
        ucs_status_ptr_t, \
        2, \
        ucp_ep_h, ep, \
        unsigned, mode)

#define MEDEV2_UCP_EP_FUNCS_EP_DESTROY(X) \
    X(ep_destroy, \
        void, \
        1, \
        ucp_ep_h, ep)

// void ucp_???(???);
#define MEDEV2_UCP_OTHER_FUNCS_SYNC_VOID(X) \
    X(config_release, \
        void, \
        1, \
        ucp_config_t *, config)  \
    X(cleanup, \
        void, \
        1, \
        ucp_context_h, context_p) \
    X(rkey_buffer_release, \
        void, \
        1, \
        void *, rkey_buffer) \
    X(rkey_destroy, \
        void, \
        1, \
        ucp_rkey_h, rkey) \
    X(request_free, \
        void, \
        1, \
        void*, request)

// ucs_status_t ucp_???(???);
#define MEDEV2_UCP_OTHER_FUNCS_SYNC_STATUS(X) \
    X(config_read, \
        ucs_status_t, \
        3, \
        const char*, env_prefix, \
        const char*, filename, \
        ucp_config_t**, config_p) \
    X(init, \
        ucs_status_t, \
        3, \
        const ucp_params_t*, params, \
        const ucp_config_t*, config, \
        ucp_context_h*, context_p) \
    X(worker_create, \
        ucs_status_t, \
        3, \
        ucp_context_h, context,\
        const ucp_worker_params_t*, params, \
        ucp_worker_h*, worker_p) \
    X(mem_map, \
        ucs_status_t, \
        3, \
        ucp_context_h, context, \
        const ucp_mem_map_params_t*, params, \
        ucp_mem_h*, memh_p) \
    X(mem_unmap, \
        ucs_status_t, \
        2, \
        ucp_context_h, context, \
        ucp_mem_h, memh) \
    X(mem_query, \
        ucs_status_t, \
        2, \
        ucp_mem_h, memh, \
        ucp_mem_attr_t*, attr) \
    X(rkey_pack, \
        ucs_status_t, \
        4, \
        ucp_context_h, context, \
        ucp_mem_h, memh, \
        void**, rkey_buffer_p, \
        size_t*, size_p) \
    X(request_check_status, \
        ucs_status_t, \
        1, \
        void*, request)


#define MEDEV2_UCP_FUNCS_ALL(X) \
    MEDEV2_UCP_WORKER_FUNCS_SYNC_VOID(X) \
    MEDEV2_UCP_WORKER_FUNCS_SYNC_STATUS(X) \
    MEDEV2_UCP_WORKER_FUNCS_EP_CREATE(X) \
    MEDEV2_UCP_WORKER_FUNCS_WORKER_PROGRESS(X) \
    MEDEV2_UCP_EP_FUNCS_SYNC_STATUS(X) \
    MEDEV2_UCP_EP_FUNCS_ASYNC_STATUS(X) \
    MEDEV2_UCP_EP_FUNCS_ASYNC_STATUS_PTR(X) \
    MEDEV2_UCP_EP_FUNCS_EP_CLOSE_NB(X) \
    MEDEV2_UCP_EP_FUNCS_EP_DESTROY(X) \
    MEDEV2_UCP_OTHER_FUNCS_SYNC_VOID(X) \
    MEDEV2_UCP_OTHER_FUNCS_SYNC_STATUS(X)


namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

#define P1(t0, a0)      t0 a0;
#define P2(t0, a0, ...) t0 a0; P1(__VA_ARGS__)
#define P3(t0, a0, ...) t0 a0; P2(__VA_ARGS__)
#define P4(t0, a0, ...) t0 a0; P3(__VA_ARGS__)
#define P5(t0, a0, ...) t0 a0; P4(__VA_ARGS__)
#define P6(t0, a0, ...) t0 a0; P5(__VA_ARGS__)
#define P7(t0, a0, ...) t0 a0; P6(__VA_ARGS__)
#define P8(t0, a0, ...) t0 a0; P7(__VA_ARGS__)

#define D(name, tr, num, ...) \
    struct name##_params { P##num(__VA_ARGS__) };

MEDEV2_UCP_FUNCS_ALL(D)

#undef D
#undef P1
#undef P2
#undef P3
#undef P4
#undef P5
#undef P6
#undef P7
#undef P8

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

