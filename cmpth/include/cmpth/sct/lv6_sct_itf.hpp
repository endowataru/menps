
#pragma once

#include <cmpth/sct/sct_mcs_mutex.hpp>
#include <cmpth/sync/basic_mcs_cv.hpp>
#include <cmpth/exec/basic_for_loop.hpp>

namespace cmpth {

// level 6: mutex, cv, for_loop

template <typename P>
struct sct_cv_policy
{
private:
    using lv5_itf_type = typename P::lv5_itf_type;
    
public:
    using mcs_mutex_type = sct_mcs_mutex<P>;
    using mcs_unique_lock_type = fdn::unique_lock<mcs_mutex_type>;
    
    using uncond_var_type       = typename lv5_itf_type::uncond_variable;
    using suspended_thread_type = typename lv5_itf_type::suspended_thread;
    using worker_type           = typename lv5_itf_type::worker;
    
    using assert_policy_type    = typename lv5_itf_type::assert_policy;
    using log_policy_type       = typename lv5_itf_type::log_policy;
};

template <typename P>
struct sct_for_loop_policy
{
private:
    using lv5_itf_type = typename P::lv5_itf_type;
    
public:
    using thread_type = typename lv5_itf_type::thread;
    
    using assert_policy_type    = typename lv5_itf_type::assert_policy;
    using log_policy_type       = typename lv5_itf_type::log_policy;
};

template <typename P>
struct lv6_sct_itf
    : P::lv5_itf_type
    , basic_for_loop<sct_for_loop_policy<P>>
{
    using mutex = sct_mcs_mutex<P>;
    using condition_variable = basic_mcs_cv<sct_cv_policy<P>>;
    
    // TODO
    using unique_mutex_lock = fdn::unique_lock<mutex>;
};

} // namespace cmpth

