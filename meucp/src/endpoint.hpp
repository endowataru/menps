
#pragma once

#include "worker.hpp"

namespace menps {
namespace meucp {

class endpoint
{
public:
    explicit endpoint(ucp_ep_h ep, worker* wk)
        : ep_(ep)
        , wk_(wk)
    { }
    
    ~endpoint() { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    ucp_ep_h real() const noexcept {
        return ep_;
    }
    
    worker* get_worker() const noexcept {
        return wk_;
    }
    
private:
    ucp_ep_h ep_;
    worker* wk_;
};

} // namespace meucp
} // namespace menps

