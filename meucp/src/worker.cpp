
#include "worker.hpp"
#include <iostream>
#include "endpoint.hpp"
#include "request.hpp"

#include "worker_thread.hpp"

extern "C"
ucs_status_t MEUCP_REAL(ucp_worker_create)(ucp_context_h context,
                               const ucp_worker_params_t *params,
                               ucp_worker_h *worker_p);
extern "C"
ucs_status_t MEUCP_WRAP(ucp_worker_create)(ucp_context_h context,
                               const ucp_worker_params_t *params,
                               ucp_worker_h *worker_p)
{
    ucp_worker_h real_wk = nullptr;
    
    const auto ret = MEUCP_REAL(ucp_worker_create)(context, params, &real_wk);
    if (ret != UCS_OK)
        return ret;
    
    *worker_p = reinterpret_cast<ucp_worker_h>(
        new menps::meucp::worker(real_wk)
    );
    
    return UCS_OK;
}

extern "C"
void MEUCP_REAL(ucp_worker_destroy)(ucp_worker_h wk);

extern "C"
void MEUCP_WRAP(ucp_worker_destroy)(ucp_worker_h wk_)
{
    auto wk = reinterpret_cast<menps::meucp::worker*>(wk_);
    
    MEUCP_REAL(ucp_worker_destroy)(wk->real());
    
    delete wk;
}

extern "C"
ucs_status_t MEUCP_WRAP(ucp_ep_create)(ucp_worker_h wk_, const ucp_ep_params_t *params,
                           ucp_ep_h *ep_p)
{
    auto wk = reinterpret_cast<menps::meucp::worker*>(wk_);
    ucp_ep_h real_ep = nullptr;
    
    const auto ret = wk->do_ep_create({ params, &real_ep });
    if (ret != UCS_OK)
        return ret;
    
    *ep_p = reinterpret_cast<ucp_ep_h>(
        new menps::meucp::endpoint(real_ep, wk)
    );
    
    return UCS_OK;
}

extern "C"
void MEUCP_WRAP(ucp_ep_destroy)(ucp_ep_h ep_)
{
    auto ep = reinterpret_cast<menps::meucp::endpoint*>(ep_);
    
    ep->get_worker()->do_ep_destroy({ ep->real() }); \
    
    delete ep;
}


extern "C"
ucs_status_ptr_t MEUCP_WRAP(ucp_disconnect_nb)(ucp_ep_h ep_)
{
    auto ep = reinterpret_cast<menps::meucp::endpoint*>(ep_);
    
    ep->get_worker()->do_disconnect_nb({ ep->real() });
    
    // Delete the "wrapper" endpoint immediately
    // even though the "wrapped" endpoint is still being destroyed.
    delete ep;
}


extern "C"
ucs_status_t MEUCP_REAL(ucp_ep_rkey_unpack)(ucp_ep_h ep_, void *rkey_buffer, ucp_rkey_h *rkey_p);

extern "C"
ucs_status_t MEUCP_WRAP(ucp_ep_rkey_unpack)(ucp_ep_h ep_, void *rkey_buffer, ucp_rkey_h *rkey_p)
{
    auto ep = reinterpret_cast<menps::meucp::endpoint*>(ep_);
    
    // Just pass to the original function.
    // TODO* Is this really thread-safe?
    return MEUCP_REAL(ucp_ep_rkey_unpack)(ep->real(), rkey_buffer, rkey_p);
}

#define C ,

#define X0(name, tr) \
    DEF_BASE(name, tr, , )
#define X1(name, tr, t0, a0) \
    DEF_BASE(name, tr, C t0 a0, C2 a0)
#define X2(name, tr, t0, a0, t1, a1) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1, C2 a0 C a1)
#define X3(name, tr, t0, a0, t1, a1, t2, a2) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2, C2 a0 C a1 C a2)
#define X4(name, tr, t0, a0, t1, a1, t2, a2, t3, a3) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3, C2 a0 C a1 C a2 C a3)
#define X5(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3 C t4 a4, C2 a0 C a1 C a2 C a3 C a4)
#define X6(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3 C t4 a4 C t5 a5, C2 a0 C a1 C a2 C a3 C a4 C a5)
#define X7(name, tr, t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
    DEF_BASE(name, tr, C t0 a0 C t1 a1 C t2 a2 C t3 a3 C t4 a4 C t5 a5 C t6 a6, C2 a0 C a1 C a2 C a3 C a4 C a5 C a6)

// Functions with worker (as first argument) doesn't require
// first member for *_params (because the worker pointer is known to the worker itself).
#define C2

#define DEF_BASE(name, tr, pars, args) \
    extern "C" \
    tr MEUCP_WRAP(ucp_ ## name)(ucp_worker_h wk_ pars) { \
        auto wk = reinterpret_cast<menps::meucp::worker*>(wk_); \
        return wk->do_ ## name({ args }); \
    }

MEUCP_WORKER_FUNCS_SYNC_STATUS_WITHOUT_EP_CREATE()
MEUCP_WORKER_FUNCS_SYNC_TAG_MESSAGE_PTR()
MEUCP_WORKER_FUNCS_SYNC_VOID()

#undef DEF_BASE

#define DEF_BASE(name, tr, pars, args) \
    extern "C" \
    tr MEUCP_WRAP(ucp_ ## name)(ucp_worker_h wk_ pars) { \
        auto wk = reinterpret_cast<menps::meucp::worker*>(wk_); \
        \
        const auto ret = wk->do_ ## name({ args }); \
        if (UCS_PTR_IS_ERR(ret)) { \
            return ret; \
        } \
        else { \
            const auto req = reinterpret_cast<ucp_request*>(ret); \
            \
            return reinterpret_cast<tr>( \
                /* TODO: use memory pool */ \
                new menps::meucp::request(req, wk) \
            ); \
        } \
    }

MEUCP_WORKER_FUNCS_SYNC_STATUS_PTR()

#undef DEF_BASE

#undef C2

#define C2 ,

#define DEF_BASE(name, tr, pars, args) \
    extern "C" \
    tr MEUCP_WRAP(ucp_ ## name)(ucp_ep_h ep_ pars) { \
        auto ep = reinterpret_cast<menps::meucp::endpoint*>(ep_); \
        return ep->get_worker()->do_ ## name({ ep->real() args }); \
    }

MEUCP_ENDPOINT_FUNCS_SYNC_STATUS()
MEUCP_ENDPOINT_FUNCS_SYNC_VOID_WITHOUT_EP_DESTROY()
MEUCP_ENDPOINT_FUNCS_SYNC_PTR_WITHOUT_DISCONNECT()

#undef DEF_BASE

#define DEF_BASE(name, tr, pars, args) \
    extern "C" \
    tr MEUCP_WRAP(ucp_ ## name)(ucp_ep_h ep_ pars) { \
        auto ep = reinterpret_cast<menps::meucp::endpoint*>(ep_); \
        ep->get_worker()->post_ ## name({ ep->real() args }); \
        return UCS_INPROGRESS; \
    }

MEUCP_ENDPOINT_FUNCS_ASYNC_STATUS()

#undef DEF_BASE

#define DEF_BASE(name, tr, pars, args) \
    extern "C" \
    tr MEUCP_WRAP(ucp_ ## name)(void* req_ pars) { \
        auto req = static_cast<menps::meucp::request*>(req_); \
        return req->get_worker()->do_ ## name({ req->real() args }); \
    }

MEUCP_REQUEST_FUNCS_SYNC_STATUS()

#undef DEF_BASE

#undef C2

#undef C
#undef X0
#undef X1
#undef X2
#undef X3
#undef X4
#undef X5
#undef X6
#undef X7

