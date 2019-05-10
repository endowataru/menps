
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

enum class ult_tag_t {
    KLT = 1
,   SMTH
,   MTH
};

#define CMPTH_ULT_HEADER_KLT    <cmpth/wrap/klt_itf.hpp>
#define CMPTH_ULT_HEADER_SMTH   <cmpth/smth/default_smth.hpp>
#define CMPTH_ULT_HEADER_MTH    <cmpth/wrap/mth_itf.hpp>

template <ult_tag_t Tag>
struct get_ult_itf_type;

template <ult_tag_t Tag>
using get_ult_itf_t = typename get_ult_itf_type<Tag>::type;

} // namespace cmpth

