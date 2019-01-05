
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/ucx/uct/uct_funcs.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_uct_endpoint
{
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    
    using size_type = typename P::size_type;
    
public:
    using proxy_iface_type = typename P::proxy_iface_type;
    using proxy_worker_type = typename P::proxy_worker_type;
    using orig_endpoint_type = typename orig_uct_itf_type::endpoint_type;
    
    explicit proxy_uct_endpoint(
        proxy_iface_type&   pr_iface
    ,   orig_endpoint_type  orig_ep
    )
        : pr_iface_(pr_iface)
        , orig_ep_(mefdn::move(orig_ep))
    {
        this->pr_iface_.add_endpoint(*this);
    }
    
    ~proxy_uct_endpoint() {
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

} // namespace meqdc
} // namespace menps

