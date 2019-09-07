
#pragma once

#include <cmpth/pool/basic_ext_return_pool.hpp>
#include <cmpth/pool/basic_numbered_pool.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/sync_tag.hpp>
#include <cmpth/prof/prof_tag.hpp>

namespace cmpth {

template <typename UltItf, sync_tag_t DelegatorTag>
struct ult_ext_itf
    : UltItf
{
    template <typename P2>
    using pool_t = basic_ext_return_pool<UltItf, P2>;
    
    template <typename P2>
    using numbered_pool_t = basic_numbered_pool<UltItf, P2>;
    
    template <typename P2>
    using delegator_t = get_delegator_t<UltItf, DelegatorTag, P2>;
    
    template <prof_tag ProfTag, typename P2>
    using prof_aspect_t = get_prof_aspect_t<ProfTag, UltItf, P2>;
};

template <ult_tag_t Tag, sync_tag_t DelegatorTag>
using get_ult_ext_itf_t = ult_ext_itf<get_ult_itf_t<Tag>, DelegatorTag>;

} // namespace cmpth

