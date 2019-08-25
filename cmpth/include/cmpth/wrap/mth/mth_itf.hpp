
#pragma once

#include <cmpth/wrap/mth/mth_thread.hpp>
#include <cmpth/wrap/mth/mth_mutex.hpp>
#include <cmpth/wrap/mth/mth_cond_var.hpp>
#include <cmpth/wrap/mth/mth_uncond_var.hpp>
#include <cmpth/wrap/mth/mth_suspended_thread.hpp>
#include <cmpth/wrap/mth/mth_thread_specific.hpp>
#include <cmpth/wrap/mth/mth_barrier.hpp>
#include <cmpth/ult_tag.hpp>
#include <cmpth/exec/basic_for_loop.hpp>
#include <cmpth/sct/def_sct_spinlock.hpp>
#include <cmpth/wrap/atomic_itf_base.hpp>
#include <cmpth/wrap/basic_wrap_worker.hpp>

//#define CMPTH_ENABLE_MTH_WORKER_CACHE

namespace cmpth {

struct mth_base_policy
{
    using assert_policy_type = def_assert_policy;
    using log_policy_type = def_log_policy;
};

// level 1

struct lv1_mth_itf
    : atomic_itf_base
{
    using spinlock = def_sct_spinlock;
    using assert_policy = mth_base_policy::assert_policy_type;
    using log_policy = mth_base_policy::log_policy_type;
    
    #ifdef CMPTH_ENABLE_MTH_WORKER_CACHE
    // FIXME: This implementation contains a bug which caches
    //        the thread-local worker number across function calls with context switching.
    //        It seems that this bug happens when this function is inlined.
    //        It is not evaluated whether __attribute__((noinline)) can solve this problem or not.
    CMPTH_NOINLINE
    static fdn::size_t get_worker_num() noexcept
    {
        static thread_local fdn::size_t wk_num_ = 0;
        auto wk_num = wk_num_;
        if (CMPTH_UNLIKELY(wk_num == 0)) {
            wk_num = static_cast<fdn::size_t>(myth_get_worker_num())+1;
            wk_num_ = wk_num;
        }
        return wk_num-1;
    }
    #else
    static fdn::size_t get_worker_num() noexcept
    {
        return myth_get_worker_num();
    }
    #endif
    static fdn::size_t get_num_workers() noexcept
    {
        return myth_get_num_workers();
    }
};

// level 3: workers

struct mth_worker_policy
    : mth_base_policy
{
    using base_ult_itf_type = lv1_mth_itf;
    using worker_num_type = fdn::size_t;
};

struct lv3_mth_itf
    : lv1_mth_itf
    , basic_wrap_worker_itf<mth_worker_policy>
{
    struct this_thread {
        static myth_thread_t native_handle() noexcept {
            return myth_self();
        }
        static void yield() {
            myth_yield();
        }
    };
};

// level 5: threads, suspended threads

struct mth_suspended_thread_policy
    : mth_base_policy
{
    using worker_type = lv3_mth_itf::worker;
};

struct lv5_mth_itf
    : lv3_mth_itf
{
    struct initializer { };
    // TODO: Call myth_init() / myth_fini() ?
    
    using thread = mth_thread<mth_base_policy>;
    using uncond_variable = mth_uncond_var;
    using suspended_thread = mth_suspended_thread<mth_suspended_thread_policy>;
    
    template <typename P>
    using thread_specific = mth_thread_specific<P>;
};

// level 6: mutex, condition variables

struct mth_for_loop_policy
    : mth_base_policy
{
    using thread_type = lv5_mth_itf::thread;
};

struct lv6_mth_itf
    : lv5_mth_itf
    , basic_for_loop<mth_for_loop_policy>
{
    using mutex = mth_mutex;
    using condition_variable = mth_cond_var;
    
    template <typename Mutex>
    using unique_lock = fdn::unique_lock<Mutex>;
    
    using unique_mutex_lock = fdn::unique_lock<mutex>;
};


// level 7: barriers

struct lv7_mth_itf
    : lv6_mth_itf
{
    using barrier = mth_barrier;
};

using mth_itf = lv7_mth_itf;

template <>
struct get_ult_itf_type<ult_tag_t::MTH>
    : fdn::type_identity<mth_itf> { };

} // namespace cmpth

