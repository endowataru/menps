
#pragma once

#include <cmpth/smth/default_smth.hpp>
#include <menps/meult/ult_itf_id.hpp>

// TODO
namespace menps {
namespace meult {

template <ult_itf_id_t UltItfId>
struct get_ult_itf_type;

template <>
struct get_ult_itf_type<ult_itf_id_t::CMPTH>
    : cmpth::fdn::type_identity<cmpth::default_smth_itf> { };

} // namespace meult
} // namespace menps

