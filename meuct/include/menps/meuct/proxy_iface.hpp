
#pragma once

#include <menps/meuct/proxy_funcs.hpp>
#include <menps/mefdn/vector.hpp>
#include <algorithm>

namespace menps {
namespace meuct {

template <typename P>
class proxy_iface
{
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    
public:
    using proxy_worker_type = typename P::proxy_worker_type;
    using proxy_endpoint_type = typename P::proxy_endpoint_type;
    using orig_iface_type = typename orig_uct_itf_type::interface_type;
    
    explicit proxy_iface(
        proxy_worker_type&  pr_wk
    ,   orig_iface_type     orig_iface
    )
        : pr_wk_(pr_wk)
        , orig_iface_(mefdn::move(orig_iface))
    {
        // FIXME
        uct_iface_progress_enable(orig_iface_.get(), UCT_PROGRESS_SEND);
    }
    
    orig_iface_type& get_orig_iface() {
        return this->orig_iface_;
    }
    proxy_worker_type& get_proxy_worker() {
        return this->pr_wk_;
    }
    
    void add_endpoint(proxy_endpoint_type& pr_ep) {
        this->pr_eps_.push_back(&pr_ep);
    }
    void remove_endpoint(proxy_endpoint_type& pr_ep) {
        const auto last =
            std::remove(this->pr_eps_.begin(), this->pr_eps_.end(), &pr_ep);
        
        this->pr_eps_.erase(last, this->pr_eps_.end());
    }
    
    void flush_ongoing() {
        for (const auto& pr_ep : this->pr_eps_) {
            pr_ep->flush_ongoing();
        }
    }
    
private:
    proxy_worker_type&  pr_wk_;
    orig_iface_type     orig_iface_;
    mefdn::vector<proxy_endpoint_type*> pr_eps_;
};

} // namespace meuct
} // namespace menps

#define MEUCT_DEFINE_PROXY_IFACE_API_PARAM(i, t, a)       t a,
#define MEUCT_DEFINE_PROXY_IFACE_API_PARAM_LAST(i, t, a)  t a
#define MEUCT_DEFINE_PROXY_IFACE_API_ARG(i, t, a)         a,
#define MEUCT_DEFINE_PROXY_IFACE_API_ARG_LAST(i, t, a)    a

#define MEUCT_DEFINE_PROXY_IFACE_API_FUNC( \
        prefix, proxy_iface_type, uf, method, name, tr, num, ...) \
    extern "C" \
    tr prefix ## name( \
        MEDEV2_UCT_EXPAND_PARAMS( \
            MEUCT_DEFINE_PROXY_IFACE_API_PARAM \
        ,   MEUCT_DEFINE_PROXY_IFACE_API_PARAM_LAST \
        ,   num, __VA_ARGS__ \
        ) \
    ) { \
        auto& pr_iface = *reinterpret_cast<proxy_iface_type*>(iface); \
        auto& pr_wk = pr_iface.get_proxy_worker(); \
        \
        return pr_wk.method ## name({ \
            MEDEV2_UCT_EXPAND_PARAMS( \
                MEUCT_DEFINE_PROXY_IFACE_API_ARG \
            ,   MEUCT_DEFINE_PROXY_IFACE_API_ARG_LAST \
            ,   num, __VA_ARGS__ \
            ) \
        }); \
    }

#define MEUCT_DEFINE_PROXY_IFACE_API(prefix, proxy_iface_type, uf) \
    extern "C" \
    ucs_status_t prefix ## iface_open( \
        const uct_md_h              md \
    ,   const uct_worker_h          worker \
    ,   const uct_iface_params_t*   params \
    ,   const uct_iface_config_t*   config \
    ,   uct_iface_h* const          iface_p \
    ) { \
        auto& pr_wk = *reinterpret_cast<proxy_iface_type::proxy_worker_type*>(worker); \
        auto& orig_wk = pr_wk.get_orig_worker(); \
        \
        auto orig_iface = \
            proxy_iface_type::orig_iface_type::open( \
                uf, md, orig_wk.get(), params, config); \
        \
        auto pr_iface = \
            new proxy_iface_type(pr_wk, menps::mefdn::move(orig_iface)); \
        \
        *iface_p = reinterpret_cast<uct_iface_h>(pr_iface); \
        \
        return UCS_OK; \
    } \
    extern "C" \
    void prefix ## iface_close(const uct_iface_h iface) { \
        auto pr_iface = reinterpret_cast<proxy_iface_type*>(iface); \
        delete pr_iface; \
    } \
    MEDEV2_UCT_IFACE_FUNCS_SYNC_STATUS( \
        MEUCT_DEFINE_PROXY_IFACE_API_FUNC, prefix, proxy_iface_type, uf, execute_)

