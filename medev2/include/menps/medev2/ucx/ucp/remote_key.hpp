
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>
#include <menps/medev2/ucx/ucx_error.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct remote_key_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type*    uf;
    
    void operator () (ucp_rkey* const p) const noexcept {
        this->uf->rkey_destroy({ p });
    }
};

template <typename P>
class remote_key
    : public mefdn::unique_ptr<ucp_rkey, remote_key_deleter<P>>
{
    using deleter_type = remote_key_deleter<P>;
    using base = mefdn::unique_ptr<ucp_rkey, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    remote_key() noexcept = default;
    
    explicit remote_key(ucp_facade_type& uf, ucp_rkey* const p)
        : base(p, deleter_type{ &uf })
    { }
    
    remote_key(const remote_key&) = delete;
    remote_key& operator = (const remote_key&) = delete;
    
    remote_key(remote_key&&) noexcept = default;
    remote_key& operator = (remote_key&&) noexcept = default;
    
    static remote_key unpack(
        ucp_facade_type&    uf
    ,   ucp_ep* const       ep
    ,   const void* const   rkey_buf
    ) {
        ucp_rkey* p = nullptr;
        
        const auto ret = uf.ep_rkey_unpack({ ep, rkey_buf, &p });
        if (ret != UCS_OK) {
            throw ucx_error("ucp_ep_rkey_unpack() failed", ret);
        }
        
        return remote_key(uf, p);
    }
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

