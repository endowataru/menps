
#pragma once

#include "worker.hpp"

extern "C" {

// This struct is defined inside UCP.
// Although there's no access privilege from our translation units to this struct,
// using this type provides better information to debuggers.
struct ucp_request;

} // extern "C"

namespace menps {
namespace meucp {

class request
{
public:
    /*implicit*/ request(ucp_request* const req, worker* const wk)
        : req_(req)
        , wk_(wk)
    { }
    
    request(const request&) = delete;
    request& operator = (const request&) = delete;
    
    ucp_request* real() const noexcept {
        return req_;
    }
    
    worker* get_worker() const noexcept {
        return wk_;
    }
    
private:
    ucp_request*    req_;
    worker*         wk_;
};

} // namespace meucp
} // namespace menps

