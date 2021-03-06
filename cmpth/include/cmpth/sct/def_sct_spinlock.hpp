
#pragma once

#include <cmpth/sync/basic_spinlock.hpp>
#include <cmpth/sync/basic_recursive_spinlock.hpp>
#include <cmpth/wrap/klt_itf.hpp>

namespace cmpth {

struct def_sct_common_policy
{
    using assert_aspect_type = def_assert_aspect;
    using log_aspect_type = def_log_aspect;
    
    using base_ult_itf_type = klt_itf;
};

struct def_sct_spinlock_policy
    : def_sct_common_policy
{
    using typename def_sct_common_policy::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
};

using def_sct_spinlock = basic_spinlock<def_sct_spinlock_policy>;

struct def_sct_recursive_spinlock_policy
    : def_sct_common_policy
{
    using base_spinlock_type = def_sct_spinlock;
};

using def_sct_recursive_spinlock = basic_recursive_spinlock<def_sct_recursive_spinlock_policy>;

} // namespace cmpth

