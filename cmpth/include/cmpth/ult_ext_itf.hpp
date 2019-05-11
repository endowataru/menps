
#pragma once

#include <cmpth/pool/basic_ext_return_pool.hpp>
#include <cmpth/pool/basic_numbered_pool.hpp>
#include <cmpth/sync/basic_pool_mcs_delegator.hpp>
#include <cmpth/ult_tag.hpp>

namespace cmpth {

template <typename UltItf>
struct ult_ext_itf
    : UltItf
{
    template <typename P2>
    using pool_t = basic_ext_return_pool<UltItf, P2>;
    
    template <typename P2>
    using numbered_pool_t = basic_numbered_pool<UltItf, P2>;
    
    template <typename P2>
    using delegator_t = pool_mcs_delegator<UltItf, P2>;
};

template <typename UltItf>
using ult_ext_itf_t = ult_ext_itf<UltItf>;

template <ult_tag_t Tag>
using get_ult_ext_itf_t = ult_ext_itf_t<get_ult_itf_t<Tag>>;

} // namespace cmpth

