
#pragma once

#include <menps/meult/common.hpp>

namespace menps {
namespace meult {

enum class ult_itf_id_t {
    KLT = 1
,   MTH
};

#define MEULT_ULT_ITF_HEADER_KLT   <menps/meult/klt/klt_policy.hpp>
#define MEULT_ULT_ITF_HEADER_MTH   <menps/meult/backend/mth/ult_policy.hpp>

template <ult_itf_id_t UltItfId>
struct get_ult_itf_type;

template <ult_itf_id_t UltItfId>
using get_ult_itf_type_t = typename get_ult_itf_type<UltItfId>::type;

} // namespace meult
} // namespace menps

