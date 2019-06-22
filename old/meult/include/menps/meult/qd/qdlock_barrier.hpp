
#pragma once

#include <menps/meult/qd/qdlock_mutex.hpp>
#include <menps/meult/qd/basic_qdlock_cv.hpp>
#include <menps/mefdn/thread/basic_barrier.hpp>
#include <menps/mefdn/mutex.hpp>

namespace menps {
namespace meult {

template <typename UltItf>
struct qdlock_cv_policy {
    using ult_itf_type = UltItf;
    using qdlock_mutex_type = qdlock_mutex<UltItf>;
    using qdlock_unique_lock_type =
        mefdn::unique_lock<qdlock_mutex_type>;
};

template <typename UltItf>
using qdlock_condition_variable = basic_qdlock_cv<qdlock_cv_policy<UltItf>>;


template <typename UltItf>
struct qdlock_barrier_policy {
    using ptrdiff_type = mefdn::ptrdiff_t;
    using mutex_type = qdlock_mutex<UltItf>;
    using cv_type = qdlock_condition_variable<UltItf>;
    using unique_lock_type = mefdn::unique_lock<mutex_type>;
};

template <typename UltItf>
using qdlock_barrier = mefdn::basic_barrier<qdlock_barrier_policy<UltItf>>;

} // namespace meult
} // namespace menps

