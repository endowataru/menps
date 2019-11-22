
#pragma once

#include <menps/medsm3/md3/lv3_dir_md3_itf.hpp>
#include <menps/medsm3/md3/lv3_ts_md3_itf.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
using lv3_md3_itf =
    fdn::conditional_t<
        P::lv2_itf_type::constants_type::use_directory_coherence
    ,   lv3_dir_md3_itf<P>
    ,   lv3_ts_md3_itf<P>
    >;

} // namespace medsm3
} // namespace menps

