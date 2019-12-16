
#pragma once

#include <cmpth/sct/sct_itf.hpp>

#include <cmpth/wss/basic_worker_tls.hpp>
#include <cmpth/wss/basic_unique_task_ptr.hpp>
#include <cmpth/sct/basic_sct_continuation.hpp>
#include <cmpth/sct/basic_sct_task_ref.hpp>
#include <cmpth/sct/basic_sct_call_stack.hpp>
#include <cmpth/sct/basic_sct_running_task.hpp>
#include <cmpth/wss/basic_worker_task.hpp>
#include <cmpth/wss/basic_worker.hpp>
#include <cmpth/wss/basic_thread_funcs.hpp>
#include <cmpth/sct/basic_sct_scheduler.hpp>
#include <cmpth/sct/basic_sct_initializer.hpp>
#include <cmpth/wss/basic_thread.hpp>
#include <cmpth/wss/basic_suspended_thread.hpp>

#include <cmpth/wrap/klt_itf.hpp>
#include <cmpth/sync/basic_spinlock.hpp>
//#include <cmpth/prof/dummy_prof_recorder.hpp>
#include <cmpth/sct/basic_sct_return_memory_pool.hpp>
#include <cmpth/sct/basic_sct_task_pool.hpp>
#include <cmpth/arch/context_policy.hpp>
#include <cmpth/tls/basic_map_tls.hpp>
#include <cmpth/sct/basic_sct_task_desc.hpp>
#include <cmpth/wd/chaselev_worker_deque.hpp>
#include <cmpth/sct/basic_sct_mcs_mutex.hpp>
#include <cmpth/sync/basic_mcs_cv.hpp>
#include <cmpth/sync/basic_cv_barrier.hpp>
#include <cmpth/exec/basic_for_loop.hpp>
#include <unordered_map>

#include <cmpth/ult_tag.hpp>

namespace cmpth {

struct def_sct_policy
{
    // Define classes provided as a top-level policy.
    using base_ult_itf_type = klt_itf;
    using constants_type = constants;
    // Aspects
    using assert_policy_type = def_assert_policy;
    using log_aspect_type = def_log_aspect;
    /*template <typename P>
    using prof_aspect_t = dummy_prof_recorder<P>;*/
    template <typename P>
    using spinlock_t = basic_spinlock<P>;
    // Contexts
    template <typename P>
    using context_policy_t = context_policy<P>;
    // Pointers to tasks
    template <typename P>
    using unique_task_ptr_t = basic_unique_task_ptr<P>;
    template <typename P>
    using task_ref_t = basic_sct_task_ref<P>;
    // Pointers to task with execution states
    template <typename P>
    using continuation_t = basic_sct_continuation<P>;
    template <typename P>
    using call_stack_t = basic_sct_call_stack<P>;
    template <typename P>
    using running_task_t = basic_sct_running_task<P>;
    // Task descriptors
    template <typename P>
    using task_desc_t = basic_sct_task_desc<P>;
    template <typename P>
    using task_mutex_t = typename P::spinlock_type;
    // Workers
    template <typename P>
    using worker_deque_t = chaselev_worker_deque<P>;
    using worker_num_type = fdn::size_t;
    static const worker_num_type invalid_worker_num = -1;
    template <typename P>
    using worker_tls_t = basic_worker_tls<P>;
    template <typename P>
    using worker_task_t = basic_worker_task<P>;
    template <typename P>
    using worker_t = basic_worker<P>;
    // Memory pools
    template <typename P>
    using memory_pool_t = basic_sct_return_memory_pool<P>;
    template <typename P>
    using task_pool_t = basic_sct_task_pool<P>;
    // Thread functions
    template <typename P>
    using thread_funcs_t = basic_thread_funcs<P>;
    // Schedulers
    template <typename P>
    using scheduler_t = basic_sct_scheduler<P>;
    template <typename P>
    using initializer_t = basic_sct_initializer<P>;
    // Threads
    template <typename P>
    using thread_t = basic_thread<P>;
    template <typename P>
    using suspended_thread_t = basic_suspended_thread<P>;
    // Synchronizations
    template <typename P>
    using mutex_t = basic_sct_mcs_mutex<P>;
    template <typename P>
    using cv_t = basic_mcs_cv<P>;
    template <typename P>
    using barrier_t = basic_cv_barrier<P>;
    // Atomics
    using atomic_itf_type = atomic_itf_base;
    // For-loops
    template <typename P>
    using for_loop_t = basic_for_loop<P>;
    // Thread-local storage (TLS)
    using tls_key_type = fdn::size_t;
    using tls_map_impl_type = std::unordered_map<tls_key_type, void*>;
    template <typename P>
    using tls_map_t = basic_map_tls_map<P>;
    template <typename P>
    using tls_key_pool_t = basic_map_tls_key_pool<P>;
    template <typename P, typename VarP>
    using thread_specific_t = basic_map_tls_thread_specific<P, VarP>;

    template <typename Pool>
    static fdn::size_t get_task_pool_threshold(Pool& /*pool*/) noexcept {
        return constants::default_task_pool_threshold;
    }
    template <typename Pool>
    static fdn::size_t get_mcs_mutex_pool_threshold(Pool& /*pool*/) noexcept {
        return constants::default_mcs_pool_threshold;
    }
    static fdn::size_t get_default_stack_size() noexcept {
        return constants::default_stack_size;
    }
    static fdn::size_t get_default_worker_deque_size() noexcept {
        return constants::default_worker_deque_size;
    }
};

using def_sct_itf = sct_itf<def_sct_policy>;

template <>
struct get_ult_itf_type<ult_tag_t::SCT>
    : fdn::type_identity<def_sct_itf> { };

} // namespace cmpth

