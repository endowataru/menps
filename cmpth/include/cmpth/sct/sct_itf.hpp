
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
struct sct_spinlock_policy {
    using assert_aspect_type = typename P::assert_aspect_type;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
};
template <typename P>
using sct_spinlock =
    typename P::template spinlock_t<sct_spinlock_policy<P>>;


// Define a opaque worker type to define a context policy.
template <typename P>
struct sct_worker;


template <typename P>
struct sct_context_policy_policy {
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_context_policy =
    typename P::template context_policy_t<sct_context_policy_policy<P>>;

template <typename P>
using sct_context =
    typename sct_context_policy<P>::template context<sct_worker<P>*>;
template <typename P>
using sct_transfer =
    typename sct_context_policy<P>::template transfer<sct_worker<P>*>;
template <typename P>
using sct_cond_transfer =
    typename sct_context_policy<P>::template cond_transfer<sct_worker<P>*>;


// Define a opaque thread descriptor type to define task pointers.
template <typename P>
struct sct_task_desc;


template <typename P>
struct sct_unique_task_ptr_policy {
    using task_desc_type = sct_task_desc<P>;
};
template <typename P>
using sct_unique_task_ptr =
    typename P::template unique_task_ptr_t<sct_unique_task_ptr_policy<P>>;


template <typename P>
struct sct_continuation_policy {
    using task_desc_type = sct_task_desc<P>;
    using unique_task_ptr_type = sct_unique_task_ptr<P>;
    using context_type = sct_context<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_continuation =
    typename P::template continuation_t<sct_continuation_policy<P>>;


template <typename P>
struct sct_task_mutex_policy {
    using spinlock_type = sct_spinlock<P>;
};
template <typename P>
using sct_task_mutex = typename P::template task_mutex_t<sct_task_mutex_policy<P>>;


template <typename P>
struct sct_task_ref_policy {
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using task_desc_type = sct_task_desc<P>;
    using task_mutex_type = sct_task_mutex<P>;
    using continuation_type = sct_continuation<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_task_ref = typename P::template task_ref_t<sct_task_ref_policy<P>>;


template <typename P>
struct sct_call_stack_policy {
    using continuation_type = sct_continuation<P>;
    using unique_task_ptr_type = sct_unique_task_ptr<P>;
    using task_desc_type = sct_task_desc<P>;
    using task_ref_type = sct_task_ref<P>;
    using context_type = sct_context<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_call_stack = typename P::template call_stack_t<sct_call_stack_policy<P>>;


template <typename P>
struct sct_tls_map_policy {
    using tls_key_type = typename P::tls_key_type;
    using tls_map_impl_type = typename P::tls_map_impl_type;
};
template <typename P>
using sct_tls_map = typename P::template tls_map_t<sct_tls_map_policy<P>>;


template <typename P>
struct sct_task_desc_policy {
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using task_mutex_type = sct_task_mutex<P>;
    using continuation_type = sct_continuation<P>;
    using context_type = sct_context<P>;
    using tls_map_type = sct_tls_map<P>;
};
template <typename P>
struct sct_task_desc
    : P::template task_desc_t<sct_task_desc_policy<P>> { };


template <typename P>
struct sct_worker_deque_policy {
    using continuation_type = sct_continuation<P>;
    using task_desc_type = sct_task_desc<P>;
    using unique_task_ptr_type = sct_unique_task_ptr<P>;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using assert_aspect_type = typename P::assert_aspect_type;
    static fdn::size_t get_default_deque_size() noexcept {
        return P::get_default_worker_deque_size();
    }
};
template <typename P>
using sct_worker_deque =
    typename P::template worker_deque_t<sct_worker_deque_policy<P>>;


template <typename P>
struct sct_running_task_policy {
    using task_desc_type = sct_task_desc<P>;
    using unique_task_ptr_type = sct_unique_task_ptr<P>;
    using task_ref_type = sct_task_ref<P>;
    using continuation_type = sct_continuation<P>;
    using context_type = sct_context<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_running_task =
    typename P::template running_task_t<sct_running_task_policy<P>>;


template <typename P>
struct sct_worker_task_policy {
    using derived_type = sct_worker<P>;
    using context_policy_type = sct_context_policy<P>;
    using context_type = sct_context<P>;
    using transfer_type = sct_transfer<P>;
    using cond_transfer_type = sct_cond_transfer<P>;

    using task_desc_type = sct_task_desc<P>;
    using task_ref_type = sct_task_ref<P>;
    using call_stack_type = sct_call_stack<P>;
    using continuation_type = sct_continuation<P>;
    using running_task_type = sct_running_task<P>;
    using log_aspect_type = typename P::log_aspect_type;
};
template <typename P>
using sct_worker_task =
    typename P::template worker_task_t<sct_worker_task_policy<P>>;


template <typename P>
struct sct_worker_tls_policy {
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using derived_type = sct_worker<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_worker_tls = typename P::template worker_tls_t<sct_worker_tls_policy<P>>;


template <typename P>
struct sct_worker_policy {
    using derived_type = sct_worker<P>;
    using worker_tls_type = sct_worker_tls<P>;
    using worker_task_type = sct_worker_task<P>;
    using worker_num_type = typename P::worker_num_type;
    using worker_deque_type = sct_worker_deque<P>;
    using call_stack_type = sct_call_stack<P>;
    using continuation_type = sct_continuation<P>;
    using task_ref_type = sct_task_ref<P>;
    using unique_task_ptr_type = sct_unique_task_ptr<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
    static const worker_num_type invalid_worker_num = P::invalid_worker_num;
};
template <typename P>
struct sct_worker : P::template worker_t<sct_worker_policy<P>> { };


template <typename P, typename P2>
struct sct_memory_pool_policy : P2 {
    using spinlock_type = sct_spinlock<P>;
    using worker_type = sct_worker<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
    using log_aspect_type = typename P::log_aspect_type;
};
template <typename P, typename P2>
using sct_memory_pool = typename P::template memory_pool_t<sct_memory_pool_policy<P, P2>>;


template <typename P>
struct sct_task_pool_policy {
    using worker_type = sct_worker<P>;
    using task_ref_type = sct_task_ref<P>;
    using call_stack_type = sct_call_stack<P>;
    using task_desc_type = sct_task_desc<P>;
    using unique_task_ptr_type = sct_unique_task_ptr<P>;
    template <typename P2>
    using memory_pool_t = sct_memory_pool<P, P2>;

    template <typename Pool>
    static fdn::size_t get_task_pool_threshold(Pool& pool) noexcept {
        return P::get_task_pool_threshold(pool);
    }
    static fdn::size_t get_default_stack_size() noexcept {
        return P::get_default_stack_size();
    }
};
template <typename P>
struct sct_task_pool
    : P::template task_pool_t<sct_task_pool_policy<P>>
{
private:
    using base = typename P::template task_pool_t<sct_task_pool_policy<P>>;

public:
    using base::base;
};


template <typename P>
struct sct_thread_funcs_policy {
    using worker_type = sct_worker<P>;
    using context_policy_type = sct_context_policy<P>;
    using transfer_type = sct_transfer<P>;
    using continuation_type = sct_continuation<P>;
    using call_stack_type = sct_call_stack<P>;
    using task_ref_type = sct_task_ref<P>;
    using task_desc_type = sct_task_desc<P>;
    using task_pool_type = sct_task_pool<P>;
};
template <typename P>
using sct_thread_funcs = typename P::template thread_funcs_t<sct_thread_funcs_policy<P>>;


template <typename P>
struct sct_tls_key_pool_policy {
    using tls_key_type = typename P::tls_key_type;
    using spinlock_type = sct_spinlock<P>;
};
template <typename P>
using sct_tls_key_pool = typename P::template tls_key_pool_t<sct_tls_key_pool_policy<P>>;


template <typename P>
struct sct_scheduler_policy;
template <typename P>
using sct_scheduler = typename P::template scheduler_t<sct_scheduler_policy<P>>;

template <typename P>
struct sct_scheduler_policy {
    using derived_type = sct_scheduler<P>;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_type = sct_worker<P>;
    using continuation_type = sct_continuation<P>;
    using task_pool_type = sct_task_pool<P>;
    using thread_funcs_type = sct_thread_funcs<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
    using tls_key_pool_type = sct_tls_key_pool<P>;
    using task_ref_type = sct_task_ref<P>;
    using log_aspect_type = typename P::log_aspect_type;
};


template <typename P>
struct sct_initializer_policy {
    using scheduler_type = sct_scheduler<P>;
};
template <typename P>
using sct_initializer = typename P::template initializer_t<sct_initializer_policy<P>>;


template <typename P>
struct sct_thread_policy;
template <typename P>
using sct_thread = typename P::template thread_t<sct_thread_policy<P>>;

template <typename P>
struct sct_thread_policy {
    using derived_type = sct_thread<P>;
    using worker_type = sct_worker<P>;
    using thread_funcs_type = sct_thread_funcs<P>;
    using task_ref_type = sct_task_ref<P>;
    using task_pool_type = sct_task_pool<P>;

    static task_pool_type& get_task_pool(worker_type& /*wk*/) {
        auto& sched = sct_scheduler<P>::get_instance();
        return sched.get_task_pool();
    }
};


template <typename P>
struct sct_suspended_thread_policy;
template <typename P>
using sct_suspended_thread =
    typename P::template suspended_thread_t<sct_suspended_thread_policy<P>>;

template <typename P>
struct sct_suspended_thread_policy {
    using derived_type = sct_suspended_thread<P>; // TODO: necessary?
    using worker_type = sct_worker<P>;
    using continuation_type = sct_continuation<P>;
    using task_desc_type = sct_task_desc<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};


template <typename P>
struct sct_mutex_policy {
    using worker_type = sct_worker<P>;
    using worker_num_type = typename P::worker_num_type;
    using suspended_thread_type = sct_suspended_thread<P>;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using assert_aspect_type = typename P::assert_aspect_type;
    using log_aspect_type = typename P::log_aspect_type;
    template <typename P2>
    using memory_pool_t = sct_memory_pool<P, P2>;
    // TODO: remove mcs
    template <typename Pool>
    static fdn::size_t get_mcs_mutex_pool_threshold(Pool& pool) {
        return P::get_mcs_mutex_pool_threshold(pool);
    }
    static fdn::size_t get_num_workers() {
        auto& sched = sct_scheduler<P>::get_instance();
        return sched.get_num_workers();
    }
};
template <typename P>
using sct_mutex = typename P::template mutex_t<sct_mutex_policy<P>>;


template <typename P>
struct sct_cv_policy {
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;

public:
    using mutex_type = sct_mutex<P>;
    using mcs_mutex_type = mutex_type; // TODO
    using mcs_unique_lock_type =
        typename base_ult_itf_type::template unique_lock<mcs_mutex_type>; // TODO
    using worker_type = sct_worker<P>;
    using suspended_thread_type = sct_suspended_thread<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
};
template <typename P>
using sct_cv = typename P::template cv_t<sct_cv_policy<P>>;


template <typename P>
struct sct_barrier_policy {
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;

public:
    using mutex_type = sct_mutex<P>;
    using cv_type = sct_cv<P>;
    using unique_lock_type =
        typename base_ult_itf_type::template unique_lock<mutex_type>;
};
template <typename P>
using sct_barrier = typename P::template barrier_t<sct_barrier_policy<P>>;


template <typename P>
struct sct_for_loop_policy {
    using thread_type = sct_thread<P>;
    using assert_aspect_type = typename P::assert_aspect_type;
    using log_aspect_type = typename P::log_aspect_type;
};
template <typename P>
using sct_for_loop = typename P::template for_loop_t<sct_for_loop_policy<P>>;


template <typename P>
struct sct_thread_specific_policy {
private:
    using tls_key_pool_type = sct_tls_key_pool<P>;
    
public:
    using worker_type = sct_worker<P>;
    using tls_map_type = sct_tls_map<P>;
    using tls_key_type = typename P::tls_key_type;
    
    static tls_key_pool_type& get_tls_key_pool() noexcept {
        auto& sched = sct_scheduler<P>::get_instance();
        return sched.get_tls_key_pool();
    }
};
template <typename P, typename VarP>
using sct_thread_specific =
    typename P::template thread_specific_t<sct_thread_specific_policy<P>, VarP>;


template <typename P>
struct sct_itf
    : P::atomic_itf_type
    , sct_for_loop<P>
{
    using spinlock = sct_spinlock<P>;

    using worker = sct_worker<P>;
    using scheduler = sct_scheduler<P>;

    using thread = sct_thread<P>;
    using suspended_thread = sct_suspended_thread<P>;

    using mutex = sct_mutex<P>;
    using condition_variable = sct_cv<P>;
    using barrier = sct_barrier<P>;

    template <typename Mutex>
    using unique_lock = fdn::unique_lock<Mutex>;
    template <typename Mutex>
    using lock_guard = fdn::lock_guard<Mutex>;

    struct this_thread {
        static void yield() {
            auto& wk = worker::get_cur_worker();
            wk.yield();
        }
        static sct_task_desc<P>* native_handle() noexcept {
            auto& wk = worker::get_cur_worker();
            const auto tk = wk.get_cur_task_ref();
            return tk.get_task_desc();
        }
    };

    using initializer = sct_initializer<P>;

    using assert_aspect = typename P::assert_aspect_type;
    using log_aspect = typename P::log_aspect_type;

    using worker_num_type = typename P::worker_num_type;
    static worker_num_type get_worker_num() noexcept {
        auto& wk = worker::get_cur_worker();
        return wk.get_worker_num();
    }
    static fdn::size_t get_num_workers() {
        auto& sched = scheduler::get_instance();
        return sched.get_num_workers();
    }
    
    template <typename VarP>
    using thread_specific = sct_thread_specific<P, VarP>;
};

} // namespace cmpth

