
#pragma once

#include <menps/meuct/proxy_funcs.hpp>

namespace menps {
namespace meuct {

template <typename P>
class proxy_endpoint
{
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    
    using size_type = typename P::size_type;
    
public:
    using proxy_iface_type = typename P::proxy_iface_type;
    using proxy_worker_type = typename P::proxy_worker_type;
    using orig_endpoint_type = typename orig_uct_itf_type::endpoint_type;
    
    explicit proxy_endpoint(
        proxy_iface_type&   pr_iface
    ,   orig_endpoint_type  orig_ep
    )
        : pr_iface_(pr_iface)
        , orig_ep_(mefdn::move(orig_ep))
    {
        this->pr_iface_.add_endpoint(*this);
    }
    
    ~proxy_endpoint() {
        this->pr_iface_.remove_endpoint(*this);
    }
    
    orig_endpoint_type& get_orig_endpoint() {
        return this->orig_ep_;
    }
    proxy_worker_type& get_proxy_worker() {
        return this->pr_iface_.get_proxy_worker();
    }
    
    void increment_ongoing() {
        ++this->num_ongoing_;
        
        auto& pr_wk = this->get_proxy_worker();
        pr_wk.add_ongoing(1);
    }
    void decrement_ongoing() {
        MEFDN_ASSERT(this->num_ongoing_ > 0);
        --this->num_ongoing_;
        
        auto& pr_wk = this->get_proxy_worker();
        pr_wk.remove_ongoing(1);
    }
    void flush_ongoing() {
        if (this->num_ongoing_ > 0) {
            auto& pr_wk = this->get_proxy_worker();
            pr_wk.remove_ongoing(this->num_ongoing_);
            
            this->num_ongoing_ = 0;
        }
    }
    
private:
    proxy_iface_type&   pr_iface_;
    orig_endpoint_type  orig_ep_;
    size_type           num_ongoing_ = 0;
};

} // namespace meuct
} // namespace menps

#define MEUCT_DEFINE_PROXY_EP_API_PARAM(i, t, a)       t a,
#define MEUCT_DEFINE_PROXY_EP_API_PARAM_LAST(i, t, a)  t a
#define MEUCT_DEFINE_PROXY_EP_API_ARG(i, t, a)         a,
#define MEUCT_DEFINE_PROXY_EP_API_ARG_LAST(i, t, a)    a

#define MEUCT_DEFINE_PROXY_EP_API_FUNC( \
        prefix, proxy_ep_type, uf, method, name, tr, num, ...) \
    extern "C" \
    tr prefix ## name( \
        MEDEV2_EXPAND_PARAMS( \
            MEUCT_DEFINE_PROXY_EP_API_PARAM \
        ,   MEUCT_DEFINE_PROXY_EP_API_PARAM_LAST \
        ,   num, __VA_ARGS__ \
        ) \
    ) { \
        auto& pr_ep = *reinterpret_cast<proxy_ep_type*>(ep); \
        auto& pr_wk = pr_ep.get_proxy_worker(); \
        \
        return pr_wk.method ## name({ \
            MEDEV2_EXPAND_PARAMS( \
                MEUCT_DEFINE_PROXY_EP_API_ARG \
            ,   MEUCT_DEFINE_PROXY_EP_API_ARG_LAST \
            ,   num, __VA_ARGS__ \
            ) \
        }); \
    }

#define MEUCT_DEFINE_PROXY_EP_API(prefix, proxy_ep_type, uf) \
    extern "C" \
    ucs_status_t prefix ## ep_create( \
        const uct_iface_h   iface \
    ,   uct_ep_h* const     ep_p \
    ) { \
        auto& pr_iface = *reinterpret_cast<proxy_ep_type::proxy_iface_type*>(iface); \
        auto& orig_iface = pr_iface.get_orig_iface(); \
        \
        auto orig_ep = proxy_ep_type::orig_endpoint_type::create(uf, orig_iface.get()); \
        \
        auto pr_ep = \
            new proxy_ep_type(pr_iface, menps::mefdn::move(orig_ep)); \
        \
        *ep_p = reinterpret_cast<uct_ep_h>(pr_ep); \
        \
        return UCS_OK; \
    } \
    extern "C" \
    void prefix ## ep_destroy(const uct_ep_h ep) { \
        const auto pr_ep = reinterpret_cast<proxy_ep_type*>(ep); \
        delete pr_ep; \
    } \
    MEDEV2_UCT_EP_FUNCS_SYNC_STATUS( \
        MEUCT_DEFINE_PROXY_EP_API_FUNC, prefix, proxy_ep_type, uf, execute_) \
    MEUCT_UCT_PROXY_EP_FUNCS_ASYNC( \
        MEUCT_DEFINE_PROXY_EP_API_FUNC, prefix, proxy_ep_type, uf, post_)

