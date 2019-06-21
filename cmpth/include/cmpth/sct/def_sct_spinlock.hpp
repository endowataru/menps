
#pragma once

#include <cmpth/sync/basic_spinlock.hpp>
#include <cmpth/wrap/klt_itf.hpp>

namespace cmpth {

struct def_sct_common_policy
{
    using assert_policy_type = def_assert_policy;
    using log_policy_type = def_log_policy;
    
    using base_ult_itf_type = klt_itf;
};

struct def_sct_spinlock_policy
    : def_sct_common_policy
{
    using typename def_sct_common_policy::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
};

using def_sct_spinlock = basic_spinlock<def_sct_spinlock_policy>;

} // namespace cmpth

