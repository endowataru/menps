
#pragma once

#include <cmpth/wrap/mth_itf.hpp>
#include <cmpth/wrap/klt_itf.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/pool/basic_ext_return_pool.hpp>
#include <cmpth/sct/basic_sct_mcs_mutex.hpp>
#include <cmpth/sync/basic_mcs_cv.hpp>
#include <cmpth/sync/basic_cv_barrier.hpp>

namespace cmpth {

struct ctmth_mutex_policy {
    using worker_type = mth_itf::worker;
    using worker_num_type = mth_itf::worker_num_type;
    using suspended_thread_type = mth_itf::suspended_thread;
    using base_ult_itf_type = klt_itf;
    using assert_aspect_type = mth_itf::assert_aspect;
    using log_aspect_type = mth_itf::log_aspect;
    template <typename P2>
    using memory_pool_t = basic_ext_return_pool<mth_itf, P2>;
    // TODO: remove mcs
    template <typename Pool>
    static fdn::size_t get_mcs_mutex_pool_threshold(Pool& /*pool*/) {
        return constants::default_mcs_pool_threshold;
    }
    static fdn::size_t get_num_workers() {
        return mth_itf::get_num_workers();
    }
};
using ctmth_mutex = basic_sct_mcs_mutex<ctmth_mutex_policy>;

struct ctmth_cv_policy {
    using mcs_mutex_type = ctmth_mutex;
    using mcs_unique_lock_type = fdn::unique_lock<ctmth_mutex>;
    using worker_type = mth_itf::worker;
    using suspended_thread_type = mth_itf::suspended_thread;
    using assert_aspect_type = mth_itf::assert_aspect;
};
using ctmth_cv = basic_mcs_cv<ctmth_cv_policy>;

struct ctmth_barrier_policy {
    using ptrdiff_type = fdn::ptrdiff_t;
    using mutex_type = ctmth_mutex;
    using cv_type = ctmth_cv;
    using unique_lock_type = fdn::unique_lock<ctmth_mutex>;
};
using ctmth_barrier = basic_cv_barrier<ctmth_barrier_policy>;

struct ctmth_for_loop_policy {
    using thread_type = mth_itf::thread;
    using assert_aspect_type = mth_itf::assert_aspect;
    using log_aspect_type = mth_itf::log_aspect;
};

struct ctmth_itf
    : lv5_mth_itf
    , basic_for_loop<ctmth_for_loop_policy>
{
    using mutex = ctmth_mutex;
    using condition_variable = ctmth_cv;
    using barrier = ctmth_barrier;
    template <typename Mutex>
    using unique_lock = fdn::unique_lock<Mutex>;
    template <typename Mutex>
    using lock_guard = fdn::lock_guard<Mutex>;
};

template <>
struct get_ult_itf_type<ult_tag_t::CTMTH>
    : fdn::type_identity<ctmth_itf> { };

} // namespace cmpth

