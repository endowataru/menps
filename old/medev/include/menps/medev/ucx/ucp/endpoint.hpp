
#pragma once

#include <menps/medev/ucx/ucp/ucp.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct endpoint_deleter
{
    void operator () (ucp_ep*) const noexcept;
};

class endpoint
    : public mefdn::unique_ptr<ucp_ep, endpoint_deleter>
{
    typedef mefdn::unique_ptr<ucp_ep, endpoint_deleter>  base;
    
public:
    endpoint() noexcept = default;
    
    explicit endpoint(ucp_ep* const p)
        : base(p)
    { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    endpoint(endpoint&&) noexcept = default;
    endpoint& operator = (endpoint&&) noexcept = default;
    
    void put(
        const void* const       buffer
    ,   const mefdn::size_t    length
    ,   const mefdn::uint64_t  remote_addr
    ,   ucp_rkey* const         rkey
    ) {
        const auto ret = ucp_put(this->get(), buffer, length, remote_addr, rkey);
        if (MEFDN_UNLIKELY(ret != UCS_OK))
            throw_error("ucp_put", ret);
    }
    
private:
    void throw_error(const char* func_name, ucs_status_t status);
};

endpoint create_endpoint(
    ucp_worker*             wk
,   const ucp_ep_params_t*  params
);

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps

