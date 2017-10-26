
#pragma once

#include "ucp_funcs.hpp"
#include "base_ult.hpp"

#define MEUCP_USE_UNCOND

#ifdef MEUCP_USE_UNCOND
    #include <menps/meult/offload/basic_uncond_offload_queue.hpp>
#else
    #include <menps/meult/offload/basic_cv_offload_queue.hpp>
#endif

#include <menps/mefdn/logger.hpp>

namespace menps {
namespace meucp {

enum class worker_command_code {

#define X0(name, tr)                                                            name,
#define X1(name, tr, t0, a0)                                                    name,
#define X2(name, tr, t0, a0, t1, a1)                                            name,
#define X3(name, tr, t0, a0, t1, a1, t2, a2)                                    name,
#define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3)                            name,
#define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4)                    name,
#define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)            name,
#define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)    name,

MEUCP_WORKER_FUNCS_SYNC_STATUS()
MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()
MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
MEUCP_WORKER_FUNCS_SYNC_VOID()
MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
MEUCP_ENDPOINT_FUNCS_SYNC_VOID()
MEUCP_ENDPOINT_FUNCS_SYNC_PTR()
MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS()
MEUCP_REQUEST_FUNCS_SYNC_STATUS()

#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
#undef X5
#undef X6
#undef X7

end

};

template <typename T>
struct notifier
{
    typename ult_policy::async_channel_<T>::type* ch;
};

struct worker_command
{
    worker_command_code code;
    
    static const mefdn::size_t cmd_size = 128 - sizeof(worker_command_code);
    char cmd[cmd_size];
};



#define X0(name, tr) \
    DEF_BASE(name, tr, )
#define X1(name, tr, t0, a0) \
    DEF_BASE(name, tr, t0 a0;)
#define X2(name, tr, t0, a0, t1, a1) \
    DEF_BASE(name, tr, t0 a0; t1 a1;)
#define X3(name, tr, t0, a0, t1, a1, t2, a2) \
    DEF_BASE(name, tr, t0 a0; t1 a1; t2 a2;)
#define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3) \
    DEF_BASE(name, tr, t0 a0; t1 a1; t2 a2; t3 a3;)
#define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4) \
    DEF_BASE(name, tr, t0 a0; t1 a1; t2 a2; t3 a3; t4 a4;)
#define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
    DEF_BASE(name, tr, t0 a0; t1 a1; t2 a2; t3 a3; t4 a4; t5 a5;)
#define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
    DEF_BASE(name, tr, t0 a0; t1 a1; t2 a2; t3 a3; t4 a4; t5 a5; t6 a6;)

#define DEF_BASE(name, tr, mems) \
    struct name ## _params { PARAMS_HEAD mems }; \
    struct name ## _command { CMD_HEAD(tr) name ## _params params; };

#define PARAMS_HEAD
#define CMD_HEAD(tr)   notifier<tr> nt;

MEUCP_WORKER_FUNCS_SYNC_STATUS()
MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()
MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
MEUCP_WORKER_FUNCS_SYNC_VOID()

#undef PARAMS_HEAD
#undef CMD_HEAD

#define PARAMS_HEAD     ucp_ep_h ep;
#define CMD_HEAD(tr)    notifier<tr> nt;

MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
MEUCP_ENDPOINT_FUNCS_SYNC_VOID()
MEUCP_ENDPOINT_FUNCS_SYNC_PTR()

#undef CMD_HEAD

#define CMD_HEAD(tr)

MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS()

#undef PARAMS_HEAD
#undef CMD_HEAD

#define PARAMS_HEAD     void* req;
#define CMD_HEAD(tr)    notifier<tr> nt;

MEUCP_REQUEST_FUNCS_SYNC_STATUS()

#undef PARAMS_HEAD
#undef CMD_HEAD

#undef DEF_BASE

#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
#undef X5
#undef X6
#undef X7

struct worker_command_queue_policy
    : meucp::ult_policy
{
    typedef worker_command     command_type;
    
    static const mefdn::size_t queue_size = 1 << 10; // TODO: magic number
};

#ifdef MEUCP_USE_UNCOND
typedef meult::basic_uncond_offload_queue<worker_command_queue_policy>
    worker_command_queue;
#else
typedef meult::basic_cv_offload_queue<worker_command_queue_policy>
    worker_command_queue;
#endif

#define C ,

#define X0(name, tr) \
    DEF_BASE(name, tr, )
#define X1(name, tr, t0, a0) \
    DEF_BASE(name, tr, C p.a0)
#define X2(name, tr, t0, a0, t1, a1) \
    DEF_BASE(name, tr, C p.a0 C p.a1)
#define X3(name, tr, t0, a0, t1, a1, t2, a2) \
    DEF_BASE(name, tr, C p.a0 C p.a1 C p.a2)
#define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3) \
    DEF_BASE(name, tr, C p.a0 C p.a1 C p.a2 C p.a3)
#define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4) \
    DEF_BASE(name, tr, C p.a0 C p.a1 C p.a2 C p.a3 C p.a4)
#define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
    DEF_BASE(name, tr, C p.a0 C p.a1 C p.a2 C p.a3 C p.a4 C p.a5)
#define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
    DEF_BASE(name, tr, C p.a0 C p.a1 C p.a2 C p.a3 C p.a4 C p.a5 C p.a6)

#define DEF_BASE(name, tr, ps) \
    inline tr call_ ## name (ucp_worker_h wk, const name ## _params& p MEFDN_MAYBE_UNUSED) { \
        MEFDN_LOG_INFO("msg:Call " #name "()."); \
        return MEUCP_REAL(ucp_ ## name)(wk ps); \
    }

MEUCP_WORKER_FUNCS_SYNC_STATUS()
MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()
MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
MEUCP_WORKER_FUNCS_SYNC_VOID()

#undef DEF_BASE

#define DEF_BASE(name, tr, ps) \
    inline tr call_ ## name (ucp_worker_h /*wk*/, const name ## _params& p MEFDN_MAYBE_UNUSED) { \
        MEFDN_LOG_INFO("msg:Call " #name "()."); \
        return MEUCP_REAL(ucp_ ## name)(p.ep ps); \
    }

MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
MEUCP_ENDPOINT_FUNCS_SYNC_PTR()
MEUCP_ENDPOINT_FUNCS_SYNC_VOID()
MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS()

#undef DEF_BASE

#define DEF_BASE(name, tr, ps) \
    inline tr call_ ## name (ucp_worker_h /*wk*/, const name ## _params& p MEFDN_MAYBE_UNUSED) { \
        MEFDN_LOG_INFO("msg:Call " #name "()."); \
        return MEUCP_REAL(ucp_ ## name)(p.req ps); \
    }

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


} // namespace meucp
} // namespace menps

