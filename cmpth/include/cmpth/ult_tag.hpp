
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

enum class ult_tag_t {
    KLT = 1
,   MTH
,   SCT
,   CTMTH
,   ABT
,   DUMMY
,   KTH
};

#define CMPTH_ULT_HEADER_KLT    <cmpth/wrap/klt_itf.hpp>
#define CMPTH_ULT_HEADER_MTH    <cmpth/wrap/mth_itf.hpp>
#define CMPTH_ULT_HEADER_SCT    <cmpth/sct/def_sct_itf.hpp>
#define CMPTH_ULT_HEADER_CTMTH  <cmpth/wrap/ctmth_itf.hpp>
#define CMPTH_ULT_HEADER_ABT    <cmpth/wrap/abt_itf.hpp>
#define CMPTH_ULT_HEADER_DUMMY  <cmpth/wrap/dummy_ult_itf.hpp>
#define CMPTH_ULT_HEADER_KTH    <cmpth/wrap/kth_itf.hpp>

template <ult_tag_t Tag>
struct get_ult_itf_type;

template <ult_tag_t Tag>
using get_ult_itf_t = typename get_ult_itf_type<Tag>::type;

} // namespace cmpth

