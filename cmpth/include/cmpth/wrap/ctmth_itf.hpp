
#pragma once

#include <cmpth/wrap/mth_itf.hpp>
#include <cmpth/wrap/klt_itf.hpp>
#include <cmpth/sct/lv6_sct_itf.hpp>
#include <cmpth/sct/lv7_sct_itf.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/wrap/basic_wrap_worker.hpp>
#include <cmpth/pool/basic_ext_return_pool.hpp>

namespace cmpth {

// level 5

struct ctmth_worker_policy {
    using base_ult_itf_type = mth_itf;
    using worker_num_type = fdn::size_t;
    using assert_policy_type = mth_itf::assert_policy;
    using log_policy_type = mth_itf::log_policy;
};

struct lv5_mth_itf
    : mth_itf
    , basic_wrap_worker_itf<ctmth_worker_policy>
{
    using base_ult_itf = klt_itf;
    
    template <typename P2>
    using pool_t = basic_ext_return_pool<mth_itf, P2>;
};

// level 6: mutex, cv, for_loop

struct lv6_ctmth_policy {
    using lv5_itf_type = lv5_mth_itf;
    
    template <typename Pool>
    static fdn::size_t get_mcs_mutex_pool_threshold(Pool& /*pool*/) noexcept {
        return constants::default_mcs_pool_threshold;
    }
};

struct lv6_ctmth_itf
    : lv6_sct_itf<lv6_ctmth_policy>
{
    // TODO: Reduce multiple base classes
    using typename basic_for_loop<sct_for_loop_policy<lv6_ctmth_policy>>::for_loop;
    using typename basic_for_loop<sct_for_loop_policy<lv6_ctmth_policy>>::for_loop_strided;
    using typename basic_for_loop<sct_for_loop_policy<lv6_ctmth_policy>>::execution;
};

// level 7: barrier

struct lv7_ctmth_policy {
    using lv6_itf_type = lv6_ctmth_itf;
};

using lv7_ctmth_itf = lv7_sct_itf<lv7_ctmth_policy>;

using ctmth_itf = lv7_ctmth_itf;

template <>
struct get_ult_itf_type<ult_tag_t::CTMTH>
    : fdn::type_identity<ctmth_itf> { };

} // namespace cmpth

