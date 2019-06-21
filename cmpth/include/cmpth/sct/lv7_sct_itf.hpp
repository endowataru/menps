
#pragma once

#include <cmpth/sync/basic_cv_barrier.hpp>

namespace cmpth {

// level 7: barrier

template <typename P>
struct sct_barrier_policy
{
private:
    using lv6_itf_type = typename P::lv6_itf_type;
    
public:
    using ptrdiff_type = fdn::ptrdiff_t;
    using mutex_type = typename lv6_itf_type::mutex;
    using cv_type = typename lv6_itf_type::condition_variable;
    using unique_lock_type = typename lv6_itf_type::unique_mutex_lock;
};

template <typename P>
struct lv7_sct_itf
    : P::lv6_itf_type
{
    using barrier = basic_cv_barrier<sct_barrier_policy<P>>;
};

} // namespace cmpth

