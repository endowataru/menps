
#include <menps/meuct/proxy_endpoint.hpp>
#include <menps/meuct/proxy_iface.hpp>
#include <menps/meuct/proxy_worker.hpp>
#include <menps/meuct/proxy_worker_thread.hpp>
#include <menps/meuct/proxy_completion_pool.hpp>
#include <menps/meuct/meuct.hpp>
#include <menps/medev2/ucx/uct/direct_facade.hpp>
#include <menps/medev2/ucx/uct/uct_policy.hpp>
#include <menps/meult/offload/basic_uncond_offload_queue.hpp>
#include <menps/meult/offload/basic_cv_offload_thread.hpp>

#include <menps/meult/backend/mth.hpp>

namespace menps {
namespace meuct {

using default_ult_itf = meult::backend::mth::ult_policy; // TODO

enum class worker_command_code {

#define D(dummy, name, tr, num, ...)    name,

MEUCT_UCT_PROXY_FUNCS(D, /*dummy*/)

#undef D

    finalize

};

struct worker_command
{
    static const mefdn::size_t params_size =
        128 - sizeof(worker_command_code) - sizeof(void*); // TODO
    
    worker_command_code code;
    void* notif;
    mefdn::byte params[params_size]; // TODO
};


struct worker_command_queue_policy
    : default_ult_itf
{
    typedef worker_command     command_type;
    
    //static const mefdn::size_t queue_size = 1 << 10; // TODO: magic number
    static const mefdn::size_t queue_size = 1 << 17; // TODO: magic number
};

using command_queue_t =
    meult::basic_uncond_offload_queue<worker_command_queue_policy>;

struct proxy_policy;

struct proxy_completion
{
    uct_completion_t    pr_comp;
        // Note: pr_comp must be first.
        // TODO: Use container_of().
    proxy_worker_thread<proxy_policy>* self;
    void*               pr_ptr;
    uct_completion_t*   orig_comp;
};

struct offload_thread_policy
    : default_ult_itf
{
    using derived_type = proxy_worker_thread<proxy_policy>;
};

struct proxy_policy
{
    using orig_uct_itf_type =
        medev2::ucx::uct::uct_policy<medev2::ucx::uct::direct_facade_policy>;
    
    using ult_itf_type = default_ult_itf;
    
    using command_type = worker_command;
    using command_code_type = worker_command_code;
    using command_queue_type = command_queue_t;
    using proxy_completion_type = proxy_completion;
    
    using proxy_worker_type = proxy_worker<proxy_policy>;
    using proxy_iface_type = proxy_iface<proxy_policy>;
    using proxy_endpoint_type = proxy_endpoint<proxy_policy>;
    using proxy_completion_pool_type = proxy_completion_pool<proxy_policy>;
    
    using worker_thread_type = proxy_worker_thread<proxy_policy>;
    using offload_thread_base_type = meult::basic_cv_offload_thread<offload_thread_policy>;
    using offload_thread_base_base_type = meult::basic_offload_thread<offload_thread_policy>;
    
    using size_type = mefdn::size_t;
};

namespace /*unnamed*/ {

proxy_policy::orig_uct_itf_type::uct_facade_type g_orig_uf;

} // unnamed namespace

} // namespace meuct
} // namespace menps

MEUCT_DEFINE_PROXY_WORKER_API(meuct_, menps::meuct::proxy_policy::proxy_worker_type, menps::meuct::g_orig_uf)
MEUCT_DEFINE_PROXY_IFACE_API(meuct_, menps::meuct::proxy_policy::proxy_iface_type, menps::meuct::g_orig_uf)
MEUCT_DEFINE_PROXY_EP_API(meuct_, menps::meuct::proxy_policy::proxy_endpoint_type, menps::meuct::g_orig_uf)

#define P(i, t, a)  t a,
#define PL(i, t, a) t a

#define A(i, t, a)  a,
#define AL(i, t, a) a

#define D(dummy, name, tr, num, ...) \
    extern "C" \
    tr meuct_ ## name ( \
        MEDEV2_EXPAND_PARAMS(P, PL, num, __VA_ARGS__) \
    ) { \
        return menps::meuct::g_orig_uf.name({ \
            MEDEV2_EXPAND_PARAMS(A, AL, num, __VA_ARGS__) \
        }); \
    }

MEUCT_UCT_NON_PROXY_FUNCS(D, /*dummy*/)

#undef D

#undef A
#undef AL

#define D(ret, name, tr, num, ...) \
    extern "C" \
    tr meuct_ ## name ( \
        MEDEV2_EXPAND_PARAMS(P, PL, num, __VA_ARGS__) \
    ) { \
        (void)iface; /* TODO: Comment out the parameter */ \
        /*menps::meuct::proxy_policy::ult_itf_type::this_thread::yield();*/ \
        return 0; \
    }

MEDEV2_UCT_IFACE_FUNCS_IFACE_PROGRESS(D, 0)

#undef D

#undef P
#undef PL


