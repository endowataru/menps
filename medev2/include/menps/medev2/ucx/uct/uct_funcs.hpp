
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>

#define MEDEV2_UCT_WORKER_FUNCS_WORKER_CREATE(X, ...) \
    X(__VA_ARGS__, \
        worker_create, \
        ucs_status_t, \
        3, \
        ucs_async_context_t *, async, \
        ucs_thread_mode_t, thread_mode, \
        uct_worker_h *, worker_p)

#define MEDEV2_UCT_WORKER_FUNCS_WORKER_DESTROY(X, ...) \
    X(__VA_ARGS__, \
        worker_destroy, \
        void, \
        1, \
        uct_worker_h, worker)

#define MEDEV2_UCT_WORKER_FUNCS_WORKER_PROGRESS(X, ...) \
    X(__VA_ARGS__, \
        worker_progress, \
        unsigned, \
        1, \
        uct_worker_h, worker)


#define MEDEV2_UCT_IFACE_FUNCS_IFACE_OPEN(X, ...) \
    X(__VA_ARGS__, \
        iface_open, \
        ucs_status_t, \
        5, \
        uct_md_h, md, \
        uct_worker_h, worker, \
        const uct_iface_params_t *, params, \
        const uct_iface_config_t *, config, \
        uct_iface_h *, iface_p)

#define MEDEV2_UCT_IFACE_FUNCS_IFACE_CLOSE(X, ...) \
    X(__VA_ARGS__, \
        iface_close, \
        void, \
        1, \
        uct_iface_h, iface)

#define MEDEV2_UCT_IFACE_FUNCS_IFACE_PROGRESS(X, ...) \
    X(__VA_ARGS__, \
        iface_progress, \
        unsigned, \
        1, \
        uct_iface_h, iface)

#define MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(X, ...) \
    X(__VA_ARGS__, \
        iface_flush, \
        ucs_status_t, \
        3, \
        uct_iface_h, iface, \
        unsigned, flags, \
        uct_completion_t *, comp)

#define MEDEV2_UCT_IFACE_FUNCS_SYNC_STATUS(X, ...) \
    X(__VA_ARGS__, \
        iface_query, \
        ucs_status_t, \
        2, \
        uct_iface_h, iface, \
        uct_iface_attr_t *, iface_attr) \
    X(__VA_ARGS__, \
        iface_get_device_address, \
        ucs_status_t, \
        2, \
        uct_iface_h, iface, \
        uct_device_addr_t *, addr) \
    X(__VA_ARGS__, \
        iface_get_address, \
        ucs_status_t, \
        2, \
        uct_iface_h, iface, \
        uct_iface_addr_t *, addr) \
    X(__VA_ARGS__, \
        iface_mem_alloc, \
        ucs_status_t, \
        5, \
        uct_iface_h, iface, \
        size_t, length, \
        unsigned, flags, \
        const char *, name, \
        uct_allocated_memory_t *, mem) \
    X(__VA_ARGS__, \
        iface_fence, \
        ucs_status_t, \
        2, \
        uct_iface_h, iface, \
        unsigned, flags) 


#define MEDEV2_UCT_EP_FUNCS_EP_CREATE(X, ...) \
    X(__VA_ARGS__, \
        ep_create, \
        ucs_status_t, \
        2, \
        uct_iface_h, iface, \
        uct_ep_h *, ep_p) \
    X(__VA_ARGS__, \
        ep_create_connected, \
        ucs_status_t, \
        4, \
        uct_iface_h, iface, \
        const uct_device_addr_t *, dev_addr, \
        const uct_iface_addr_t *, iface_addr, \
        uct_ep_h *, ep_p)

#define MEDEV2_UCT_EP_FUNCS_EP_DESTROY(X, ...) \
    X(__VA_ARGS__, \
        ep_destroy, \
        void, \
        1, \
        uct_ep_h, ep)

#define MEDEV2_UCT_EP_FUNCS_EP_FLUSH(X, ...) \
    X(__VA_ARGS__, \
        ep_flush, \
        ucs_status_t, \
        3, \
        uct_ep_h, ep, \
        unsigned, flags, \
        uct_completion_t *, comp)

#define MEDEV2_UCT_EP_FUNCS_SYNC_STATUS(X, ...) \
    X(__VA_ARGS__, \
        ep_get_address, \
        ucs_status_t, \
        2, \
        uct_ep_h, ep, \
        uct_ep_addr_t *, addr) \
    X(__VA_ARGS__, \
        ep_connect_to_ep, \
        ucs_status_t, \
        3, \
        uct_ep_h, ep, \
        const uct_device_addr_t *, dev_addr, \
        const uct_ep_addr_t *, ep_addr)

#define MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(X, ...) \
    X(__VA_ARGS__, \
        ep_put_short, \
        ucs_status_t, \
        5, \
        uct_ep_h, ep, \
        const void *, buffer, \
        unsigned, length, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey) \
    X(__VA_ARGS__, \
        ep_get_short, \
        ucs_status_t, \
        5, \
        uct_ep_h, ep, \
        void *, buffer, \
        unsigned, length, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey) \
    X(__VA_ARGS__, \
        ep_fence, \
        ucs_status_t, \
        2, \
        uct_ep_h, ep, \
        unsigned, flags)

#define MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(X, ...) \
    X(__VA_ARGS__, \
        ep_put_zcopy, \
        ucs_status_t, \
        6, \
        uct_ep_h, ep, \
        const uct_iov_t *, iov, \
        size_t, iovcnt, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey, \
        uct_completion_t *, comp) \
    X(__VA_ARGS__, \
        ep_get_bcopy, \
        ucs_status_t, \
        7, \
        uct_ep_h, ep, \
        uct_unpack_callback_t, unpack_cb, \
        void *, arg, \
        size_t, length, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey, \
        uct_completion_t *, comp) \
    X(__VA_ARGS__, \
        ep_get_zcopy, \
        ucs_status_t, \
        6, \
        uct_ep_h, ep, \
        const uct_iov_t *, iov, \
        size_t, iovcnt, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey, \
        uct_completion_t *, comp) \
    X(__VA_ARGS__, \
        ep_atomic_cswap64, \
        ucs_status_t, \
        7, \
        uct_ep_h, ep, \
        uint64_t, compare, \
        uint64_t, swap, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey, \
        uint64_t *, result, \
        uct_completion_t *, comp)

#define MEDEV2_UCT_EP_FUNCS_ASYNC_SSIZE_T_IMPLICIT(X, ...) \
    X(__VA_ARGS__, \
        ep_put_bcopy, \
        ssize_t, \
        5, \
        uct_ep_h, ep, \
        uct_pack_callback_t, pack_cb, \
        void *, arg, \
        uint64_t, remote_addr, \
        uct_rkey_t, rkey)


#define MEDEV2_UCT_OTHER_FUNCS_SYNC_STATUS(X, ...) \
    X(__VA_ARGS__, \
        query_md_resources, \
        ucs_status_t, \
        2, \
        uct_md_resource_desc_t **, resources_p, \
        unsigned *, num_resources_p) \
    X(__VA_ARGS__, \
        md_open, \
        ucs_status_t, \
        3, \
        const char *, md_name, \
        const uct_md_config_t *, config, \
        uct_md_h *, md_p) \
    X(__VA_ARGS__, \
        md_query_tl_resources, \
        ucs_status_t, \
        3, \
        uct_md_h, md, \
        uct_tl_resource_desc_t **, resources_p, \
        unsigned *, num_resources_p) \
    X(__VA_ARGS__, \
        md_iface_config_read, \
        ucs_status_t, \
        5, \
        uct_md_h, md, \
        const char *, tl_name, \
        const char *, env_prefix, \
        const char *, filename, \
        uct_iface_config_t **, config_p) \
    X(__VA_ARGS__, \
        config_modify, \
        ucs_status_t, \
        3, \
        void*, config, \
        const char*, name,  \
        const char*, value) \
    X(__VA_ARGS__, \
        md_query, \
        ucs_status_t, \
        2, \
        uct_md_h, md, \
        uct_md_attr_t *, md_attr) \
    X(__VA_ARGS__, \
        md_mem_alloc, \
        ucs_status_t, \
        6, \
        uct_md_h, md, \
        size_t *, length_p, \
        void **, address_p, \
        unsigned, flags, \
        const char *, name, \
        uct_mem_h *, memh_p) \
    X(__VA_ARGS__, \
        md_mem_free, \
        ucs_status_t, \
        2, \
        uct_md_h, md, \
        uct_mem_h, memh) \
    X(__VA_ARGS__, \
        md_mem_advise, \
        ucs_status_t, \
        5, \
        uct_md_h, md, \
        uct_mem_h, memh, \
        void *, addr, \
        size_t, length, \
        uct_mem_advice_t, advice) \
    X(__VA_ARGS__, \
        md_mem_reg, \
        ucs_status_t, \
        5, \
        uct_md_h, md, \
        void *, address, \
        size_t, length, \
        unsigned, flags, \
        uct_mem_h *, memh_p) \
    X(__VA_ARGS__, \
        md_mem_dereg, \
        ucs_status_t, \
        2, \
        uct_md_h, md, \
        uct_mem_h, memh) \
    X(__VA_ARGS__, \
        mem_alloc, \
        ucs_status_t, \
        9, \
        void *, addr, \
        size_t, min_length, \
        unsigned, flags, \
        uct_alloc_method_t *, methods, \
        unsigned, num_methods, \
        uct_md_h *, mds, \
        unsigned, num_mds, \
        const char *, name, \
        uct_allocated_memory_t *, mem) \
    X(__VA_ARGS__, \
        mem_free, \
        ucs_status_t, \
        1, \
        const uct_allocated_memory_t *, mem) \
    X(__VA_ARGS__, \
        md_config_read, \
        ucs_status_t, \
        4, \
        const char *, name, \
        const char *, env_prefix, \
        const char *, filename, \
        uct_md_config_t **, config_p) \
    X(__VA_ARGS__, \
        md_mkey_pack, \
        ucs_status_t, \
        3, \
        uct_md_h, md, \
        uct_mem_h, memh, \
        void *, rkey_buffer) \
    X(__VA_ARGS__, \
        rkey_unpack, \
        ucs_status_t, \
        2, \
        const void *, rkey_buffer, \
        uct_rkey_bundle_t *, rkey_ob) \
    X(__VA_ARGS__, \
        rkey_ptr, \
        ucs_status_t, \
        3, \
        uct_rkey_bundle_t *, rkey_ob, \
        uint64_t, remote_addr, \
        void **, addr_p) \
    X(__VA_ARGS__, \
        rkey_release, \
        ucs_status_t, \
        1, \
        const uct_rkey_bundle_t *, rkey_ob)

#define MEDEV2_UCT_OTHER_FUNCS_SYNC_VOID(X, ...) \
    X(__VA_ARGS__, \
        release_md_resource_list, \
        void, \
        1, \
        uct_md_resource_desc_t *, resources) \
    X(__VA_ARGS__, \
        md_close, \
        void, \
        1, \
        uct_md_h, md) \
    X(__VA_ARGS__, \
        release_tl_resource_list, \
        void, \
        1, \
        uct_tl_resource_desc_t *, resources) \
    X(__VA_ARGS__, \
        config_release, \
        void, \
        1, \
        void *, config) \
    X(__VA_ARGS__, \
        iface_mem_free, /* TODO */ \
        void, \
        1, \
        const uct_allocated_memory_t *, mem)


#define MEDEV2_UCT_FUNCS_ALL(X, ...) \
    MEDEV2_UCT_WORKER_FUNCS_WORKER_CREATE(X, __VA_ARGS__) \
    MEDEV2_UCT_WORKER_FUNCS_WORKER_DESTROY(X, __VA_ARGS__) \
    MEDEV2_UCT_WORKER_FUNCS_WORKER_PROGRESS(X, __VA_ARGS__) \
    MEDEV2_UCT_IFACE_FUNCS_IFACE_OPEN(X, __VA_ARGS__) \
    MEDEV2_UCT_IFACE_FUNCS_IFACE_CLOSE(X, __VA_ARGS__) \
    MEDEV2_UCT_IFACE_FUNCS_IFACE_PROGRESS(X, __VA_ARGS__) \
    MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(X, __VA_ARGS__) \
    MEDEV2_UCT_IFACE_FUNCS_SYNC_STATUS(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_EP_CREATE(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_EP_DESTROY(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_EP_FLUSH(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_SYNC_STATUS(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(X, __VA_ARGS__) \
    MEDEV2_UCT_EP_FUNCS_ASYNC_SSIZE_T_IMPLICIT(X, __VA_ARGS__) \
    MEDEV2_UCT_OTHER_FUNCS_SYNC_STATUS(X, __VA_ARGS__) \
    MEDEV2_UCT_OTHER_FUNCS_SYNC_VOID(X, __VA_ARGS__)


#define MEDEV2_UCT_EXPAND_PARAMS_1(i, X, XL, t0, a0)      XL(i, t0, a0)
#define MEDEV2_UCT_EXPAND_PARAMS_2(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_1(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_3(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_2(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_4(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_3(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_5(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_4(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_6(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_5(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_7(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_6(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_8(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_7(i+1, X, XL, __VA_ARGS__)
#define MEDEV2_UCT_EXPAND_PARAMS_9(i, X, XL, t0, a0, ...) X(i, t0, a0) MEDEV2_UCT_EXPAND_PARAMS_8(i+1, X, XL, __VA_ARGS__)

#define MEDEV2_UCT_EXPAND_PARAMS(X, XL, num, ...) \
    MEDEV2_UCT_EXPAND_PARAMS_ ## num(0, X, XL, __VA_ARGS__)


namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

#define P(i, t, a)  t a;

#define D(dummy, name, tr, num, ...) \
    struct name##_params { \
        MEDEV2_UCT_EXPAND_PARAMS(P, P, num, __VA_ARGS__) \
    };

MEDEV2_UCT_FUNCS_ALL(D, /*dummy*/)

#undef D
#undef P

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

