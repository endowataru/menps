
#pragma once

#include <menps/medev2/common.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

enum class uct_itf_id_t
{
    DIRECT = 1
,   QDC
};

#define MEDEV2_UCT_ITF_HEADER_DIRECT    <menps/medev2/ucx/uct/uct_policy.hpp>
#define MEDEV2_UCT_ITF_HEADER_QDC       <menps/meqdc/ucx/uct/proxy_uct_itf.hpp>

template <uct_itf_id_t UctItfId, typename P>
struct get_uct_itf_type;

template <uct_itf_id_t UctItfId, typename P>
using get_uct_itf_type_t = typename get_uct_itf_type<UctItfId, P>::type;

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

