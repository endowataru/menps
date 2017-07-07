
#pragma once

#include <mgdev/ucx/ucp/ucp.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct endpoint_deleter
{
    void operator () (ucp_ep*) const MGBASE_NOEXCEPT;
};

class endpoint
    : public mgbase::unique_ptr<ucp_ep, endpoint_deleter>
{
    typedef mgbase::unique_ptr<ucp_ep, endpoint_deleter>  base;
    
public:
    endpoint() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit endpoint(ucp_ep* const p)
        : base(p)
    { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(endpoint, base)
    
    void put(
        const void* const       buffer
    ,   const mgbase::size_t    length
    ,   const mgbase::uint64_t  remote_addr
    ,   ucp_rkey* const         rkey
    ) {
        const auto ret = ucp_put(this->get(), buffer, length, remote_addr, rkey);
        if (MGBASE_UNLIKELY(ret != UCS_OK))
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
} // namespace mgdev

