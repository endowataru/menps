
#pragma once

#include <menps/meqdc/ucx/uct/proxy_uct_facade.hpp>
#include <menps/meqdc/ucx/uct/proxy_uct_worker.hpp>
#include <menps/meuct/proxy_iface.hpp>
#include <menps/meuct/proxy_endpoint.hpp>
#include <menps/meuct/proxy_completion_pool.hpp>
#include <menps/meult/qd/qdlock_delegator.hpp>
#include <menps/medev2/ucx/uct/uct_policy.hpp>

namespace menps {
namespace meqdc {

union proxy_uct_params {
    #define D(dummy, name, tr, num, ...) \
        medev2::ucx::uct::name##_params name; \
    
    // async (ep, post, implicit)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(D, /*dummy*/)
    // async (ep, post completion)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(D, /*dummy*/)
    // async (iface_flush)
    MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(D, /*dummy*/)
    // async (ep_flush)
    MEDEV2_UCT_EP_FUNCS_EP_FLUSH(D, /*dummy*/)
    
    #undef D
};

enum class proxy_uct_code {
    inv_op = 0,
    
    #define D(dummy, name, tr, num, ...) \
        name,
    
    // async (ep, post, implicit)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(D, /*dummy*/)
    // async (ep, post completion)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(D, /*dummy*/)
    // async (iface_flush)
    MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(D, /*dummy*/)
    // async (ep_flush)
    MEDEV2_UCT_EP_FUNCS_EP_FLUSH(D, /*dummy*/)
    
    #undef D
};

struct proxy_uct_qdlock_func
{
    proxy_uct_code          code;
    proxy_uct_params        params;
};

template <typename P>
struct proxy_uct_completion
{
private:
    using proxy_worker_type = typename P::proxy_worker_type;
    
public:
    uct_completion_t    pr_comp;
    proxy_worker_type*  self;
    void*               pr_ptr;
    uct_completion_t*   orig_comp;
};

template <typename OrigUctItf, typename UltItf>
struct proxy_itf_policy
{
    using orig_uct_itf_type = OrigUctItf;
    using proxy_uct_facade_type = proxy_uct_facade<proxy_itf_policy>;
    using proxy_worker_type = proxy_uct_worker<proxy_itf_policy>;
    using proxy_iface_type = meuct::proxy_iface<proxy_itf_policy>;
    using proxy_endpoint_type = meuct::proxy_endpoint<proxy_itf_policy>;
    
    using proxy_completion_type = proxy_uct_completion<proxy_itf_policy>;
    using proxy_completion_pool_type =
        meuct::proxy_completion_pool<proxy_itf_policy>;
    
    using ult_itf_type = UltItf;
    using size_type = mefdn::size_t;
    
    using command_code_type = proxy_uct_code;
    using proxy_params_type = proxy_uct_params;
    
    using qdlock_delegator_type =
        meult::qdlock_delegator<proxy_uct_qdlock_func, ult_itf_type>;
};

template <typename OrigUctItf, typename UltItf>
struct proxy_uct_itf_policy
{
    using uct_facade_type =
        proxy_uct_facade<proxy_itf_policy<OrigUctItf, UltItf>>;
};

template <typename OrigUctItf, typename UltItf>
struct proxy_uct_itf
    : medev2::ucx::uct::uct_policy<proxy_uct_itf_policy<OrigUctItf, UltItf>>
{
    using ult_itf_type = UltItf;
};

} // namespace meqdc

namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct get_uct_itf_type<uct_itf_id_t::QDC, P>
    : mefdn::type_identity<
        meqdc::proxy_uct_itf<
            medev2::ucx::uct::direct_uct_itf<typename P::ult_itf_type>
        ,   typename P::ult_itf_type
        >
    >
{ };

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps
