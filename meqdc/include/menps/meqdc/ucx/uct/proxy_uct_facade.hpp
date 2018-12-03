
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/ucx/uct/uct_funcs.hpp>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_uct_facade
{
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    using orig_uct_facade_type = typename orig_uct_itf_type::uct_facade_type;
    
    using proxy_worker_type = typename P::proxy_worker_type;
    using proxy_iface_type = typename P::proxy_iface_type;
    using proxy_endpoint_type = typename P::proxy_endpoint_type;
    
public:
    proxy_uct_facade() = default;
    
    ucs_status_t worker_create(const medev2::ucx::uct::worker_create_params& p)
    {
        auto orig_wk =
            proxy_worker_type::orig_worker_type::create(
                this->orig_uf_, p.async, p.thread_mode);
        
        const auto pr_wk =
            new proxy_worker_type(this->orig_uf_, menps::mefdn::move(orig_wk));
        
        *p.worker_p = reinterpret_cast<uct_worker_h>(pr_wk);
        
        return UCS_OK;
    }
    
    void worker_destroy(const medev2::ucx::uct::worker_destroy_params& p)
    {
        auto pr_wk = reinterpret_cast<proxy_worker_type*>(p.worker);
        
        delete pr_wk;
    }
    
    ucs_status_t iface_open(const medev2::ucx::uct::iface_open_params& p)
    {
        auto& pr_wk = *reinterpret_cast<proxy_worker_type*>(p.worker);
        auto& orig_wk = pr_wk.get_orig_worker();
       
        auto orig_iface =
            proxy_iface_type::orig_iface_type::open(
                this->orig_uf_, p.md, orig_wk.get(), p.params, p.config);
       
        auto pr_iface =
            new proxy_iface_type(pr_wk, menps::mefdn::move(orig_iface));
       
        *p.iface_p = reinterpret_cast<uct_iface_h>(pr_iface);
       
        return UCS_OK;
    }
    
    void iface_close(const medev2::ucx::uct::iface_close_params& p)
    {
        auto pr_iface = reinterpret_cast<proxy_iface_type*>(p.iface);
        delete pr_iface;
    }
    
    ucs_status_t ep_create(const medev2::ucx::uct::ep_create_params& p)
    {
        auto& pr_iface = *reinterpret_cast<proxy_iface_type*>(p.iface);
        auto& orig_iface = pr_iface.get_orig_iface();
       
        auto orig_ep =
            proxy_endpoint_type::orig_endpoint_type::create(
                this->orig_uf_, orig_iface.get());
       
        auto pr_ep =
            new proxy_endpoint_type(pr_iface, menps::mefdn::move(orig_ep));
       
        *p.ep_p = reinterpret_cast<uct_ep_h>(pr_ep);
       
        return UCS_OK;
    }
    
    void ep_destroy(const medev2::ucx::uct::ep_destroy_params& p)
    {
        const auto pr_ep = reinterpret_cast<proxy_endpoint_type*>(p.ep);
        delete pr_ep;
    }
    
private:
    template <typename Params>
    proxy_worker_type& get_proxy_worker_from_iface(const Params& p) {
        const auto pr_iface = reinterpret_cast<proxy_iface_type*>(p.iface);
        return pr_iface->get_proxy_worker();
    }
    template <typename Params>
    proxy_worker_type& get_proxy_worker_from_ep(const Params& p) {
        const auto pr_ep = reinterpret_cast<proxy_endpoint_type*>(p.ep);
        return pr_ep->get_proxy_worker();
    }
    
public:
    #define D(get_proxy_worker, name, tr, num, ...) \
        tr name(const medev2::ucx::uct::name ## _params& p) { \
            auto& pr_wk = get_proxy_worker(p); \
            return pr_wk.post_ ## name(p); \
        }
    
    MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(D, get_proxy_worker_from_iface)
    MEDEV2_UCT_IFACE_FUNCS_SYNC_STATUS(D, get_proxy_worker_from_iface)
    
    MEDEV2_UCT_EP_FUNCS_EP_FLUSH(D, get_proxy_worker_from_ep)
    MEDEV2_UCT_EP_FUNCS_SYNC_STATUS(D, get_proxy_worker_from_ep)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(D, get_proxy_worker_from_ep)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(D, get_proxy_worker_from_ep)
    
    #undef D
    
    
    #define D(dummy, name, tr, num, ...) \
        tr name(const medev2::ucx::uct::name ## _params& p) { \
            return this->orig_uf_.name(p); \
        }
    
    MEDEV2_UCT_OTHER_FUNCS_SYNC_STATUS(D, /*dummy*/)
    MEDEV2_UCT_OTHER_FUNCS_SYNC_VOID(D, /*dummy*/)
    
    #undef D
    
private:
    orig_uct_facade_type orig_uf_;
};

} // namespace meqdc
} // namespace menps

