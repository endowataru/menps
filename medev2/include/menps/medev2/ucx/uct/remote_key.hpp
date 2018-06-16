
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/scope/basic_unique_resource.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct remote_key_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type*    uf;
    
    void operator () (const uct_rkey_bundle_t& rkey_bundle) const noexcept {
        uf->rkey_release({ &rkey_bundle });
    }
};

template <typename P>
class remote_key;

template <typename P>
struct remote_key_policy
{
    using derived_type = remote_key<P>;
    using deleter_type = remote_key_deleter<P>;
    using resource_type = uct_rkey_bundle_t;
};

template <typename P>
class remote_key
    : public mefdn::basic_unique_resource<remote_key_policy<P>>
{
    using policy_type = remote_key_policy<P>;
    using base = mefdn::basic_unique_resource<policy_type>;
    using deleter_type = typename policy_type::deleter_type;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    remote_key() MEFDN_DEFAULT_NOEXCEPT = default;
    
    explicit remote_key(
        uct_facade_type&    uf
    ,   uct_rkey_bundle_t   rkey_bundle
    )
        : base(mefdn::move(rkey_bundle), deleter_type{ &uf })
    { }
    
    remote_key(const remote_key&) = delete;
    remote_key& operator = (const remote_key&) = delete;
    
    remote_key(remote_key&&) MEFDN_DEFAULT_NOEXCEPT = default;
    remote_key& operator = (remote_key&&) MEFDN_DEFAULT_NOEXCEPT = default;
    
    static remote_key unpack(
        uct_facade_type&    uf
    ,   const void* const   rkey_buf
    ) {
        uct_rkey_bundle_t rkey_bundle = uct_rkey_bundle_t();
        
        const auto ret = uf.rkey_unpack({ rkey_buf, &rkey_bundle });
        if (ret != UCS_OK) {
            throw ucx_error("uct_rkey_unpack() failed", ret);
        }
        
        return remote_key(uf, rkey_bundle);
    }
    
private:
    friend mefdn::basic_unique_resource_access;
    
    bool is_owned() const noexcept {
        return this->get_resource().rkey != 0;
    }
    
    void set_unowned() noexcept {
        this->get_resource() = uct_rkey_bundle_t();
    }
    void set_owned() noexcept {
        // do nothing
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps
