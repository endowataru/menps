
#pragma once

#include <cmpth/sync/basic_pool_mcs_delegator.hpp>
#include <cmpth/sync/basic_ring_buf_delegator.hpp>
// TODO: remove unnecessary dependencies

namespace cmpth {

enum class sync_tag_t {
    MCS = 1
,   RB
};

template <typename UltItf, sync_tag_t DelegatorTag, typename P2>
struct get_delegator_type;

template <typename UltItf, sync_tag_t DelegatorTag, typename P2>
using get_delegator_t = typename get_delegator_type<UltItf, DelegatorTag, P2>::type;

template <typename UltItf, typename P2>
struct get_delegator_type<UltItf, sync_tag_t::MCS, P2>
    : fdn::type_identity<pool_mcs_delegator<UltItf, P2>> {};

template <typename UltItf, typename P2>
struct get_delegator_type<UltItf, sync_tag_t::RB, P2>
    : fdn::type_identity<ring_buf_delegator<UltItf, P2>> {};

} // namespace cmpth

