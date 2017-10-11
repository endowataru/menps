
#pragma once

#include <mgdev/ucx/ucp/ucp.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdev {
namespace ucx {
namespace ucp {

struct remote_key_deleter
{
    void operator () (ucp_rkey*) const MGBASE_NOEXCEPT;
};

class remote_key
    : public mgbase::unique_ptr<ucp_rkey, remote_key_deleter>
{
    typedef mgbase::unique_ptr<ucp_rkey, remote_key_deleter>  base;
    
public:
    remote_key() MGBASE_DEFAULT_NOEXCEPT = default;
    
    explicit remote_key(ucp_rkey* const p)
        : base(p)
    { }
    
    remote_key(const remote_key&) = delete;
    remote_key& operator = (const remote_key&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(remote_key, base)
};

remote_key unpack_rkey(
    ucp_ep* ep
,   void*   rkey_buf
);

} // namespace ucp
} // namespace ucx
} // namespace mgdev
