
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/ucx/uct/uct_funcs.hpp>
#include <algorithm>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_uct_iface
{
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    
public:
    using proxy_worker_type = typename P::proxy_worker_type;
    using proxy_endpoint_type = typename P::proxy_endpoint_type;
    using orig_iface_type = typename orig_uct_itf_type::interface_type;
    
    explicit proxy_uct_iface(
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
    std::vector<proxy_endpoint_type*> pr_eps_;
};

} // namespace meqdc
} // namespace menps

