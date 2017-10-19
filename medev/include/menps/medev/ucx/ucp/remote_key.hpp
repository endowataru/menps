
#pragma once

#include <menps/medev/ucx/ucp/ucp.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medev {
namespace ucx {
namespace ucp {

struct remote_key_deleter
{
    void operator () (ucp_rkey*) const noexcept;
};

class remote_key
    : public mefdn::unique_ptr<ucp_rkey, remote_key_deleter>
{
    typedef mefdn::unique_ptr<ucp_rkey, remote_key_deleter>  base;
    
public:
    remote_key() noexcept = default;
    
    explicit remote_key(ucp_rkey* const p)
        : base(p)
    { }
    
    remote_key(const remote_key&) = delete;
    remote_key& operator = (const remote_key&) = delete;
    
    remote_key(remote_key&&) noexcept = default;
    remote_key& operator = (remote_key&&) noexcept = default;
};

remote_key unpack_rkey(
    ucp_ep* ep
,   void*   rkey_buf
);

} // namespace ucp
} // namespace ucx
} // namespace medev
} // namespace menps
