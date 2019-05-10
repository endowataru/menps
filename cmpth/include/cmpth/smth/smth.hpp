
#pragma once

#include <cmpth/wss/basic_worker.hpp>
#include <cmpth/wss/basic_worker_task.hpp>
#include <cmpth/wss/basic_worker_tls.hpp>
#include <cmpth/wss/basic_unique_task_ptr.hpp>
#include <cmpth/wss/basic_thread_funcs.hpp>
#include <cmpth/wss/basic_thread.hpp>
#include <cmpth/smth/smth_worker_deque.hpp>
#include <cmpth/smth/smth_call_stack.hpp>
#include <cmpth/smth/smth_continuation.hpp>
#include <cmpth/smth/smth_running_task.hpp>
#include <cmpth/smth/smth_task_ref.hpp>
#include <cmpth/smth/basic_smth_scheduler.hpp>
#include <cmpth/sync/basic_mcs_core.hpp>
#include <cmpth/sync/basic_mcs_spinlock.hpp>
#include <cmpth/sync/basic_mcs_mutex.hpp>
#include <cmpth/sync/basic_mcs_cv.hpp>
#include <cmpth/sync/basic_cv_barrier.hpp>
#include <cmpth/sync/basic_spinlock.hpp>
#include <cmpth/sync/basic_uncond_var.hpp>
#include <cmpth/sync/basic_pool_mcs_mutex.hpp>
#include <cmpth/tls/basic_map_tls.hpp>
#include <cmpth/exec/basic_for_loop.hpp>

namespace cmpth {

template <typename P>
struct smth_task_desc
{
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
    
    using task_mutex_type = typename P::task_mutex_type;
    
    using continuation_type = typename P::continuation_type;
    
    using context_type = typename P::context_type;
    
    using tls_map_type = typename P::tls_map_type;
    
public:
    task_mutex_type     mtx;
    atomic_bool_type    finished;
    bool                detached;
    continuation_type   joiner;
    void*               stk_top;
    void*               stk_bottom;
    context_type        ctx;
    tls_map_type        tls;
};


template <typename P>
struct smth_worker_policy;

template <typename P>
struct smth_base_policy
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using context_policy_type = typename P::context_policy_type;
    using task_mutex_type = typename P::task_mutex_type;
    
    using assert_policy_type = typename P::assert_policy_type;
    using log_policy_type = typename P::log_policy_type;
    
    using task_desc_type = smth_task_desc<smth_base_policy>;
    using task_ref_type = smth_task_ref<smth_base_policy>;
    
    using call_stack_type = smth_call_stack<smth_base_policy>;
    using continuation_type = smth_continuation<smth_base_policy>;
    
    using unique_task_ptr_type = basic_unique_task_ptr<smth_base_policy>;
    
    using worker_type = basic_worker<smth_worker_policy<P>>;
    using worker_num_type = fdn::int32_t;
    
    using context_type =
        typename context_policy_type::template context<worker_type*>;
    using transfer_type =
        typename context_policy_type::template transfer<worker_type*>;
    
    using tls_map_type = basic_map_tls_map<smth_base_policy>;
    using tls_key_type = typename P::tls_key_type;
    using tls_map_impl_type = typename P::tls_map_impl_type;
};

template <typename P>
struct smth_worker_policy
    : smth_base_policy<P>
{
    using derived_type = basic_worker<smth_worker_policy>;
    
    using worker_task_type = basic_worker_task<smth_worker_policy>;
    using running_task_type = smth_running_task<smth_base_policy<P>>;
    using worker_tls_type = basic_worker_tls<smth_worker_policy>;
    
    using worker_deque_type = typename P::worker_deque_type;
    
    using typename smth_base_policy<P>::worker_num_type;
    static const worker_num_type invalid_worker_num = -1;
};

template <typename P>
struct smth_thread_policy
    : smth_base_policy<P>
{
    using derived_type = basic_thread<smth_thread_policy>;
    
    using thread_funcs_type = basic_thread_funcs<smth_thread_policy>;
    
    using typename smth_base_policy<P>::worker_type;
    using task_pool_type =
        typename P::template task_pool_t<smth_base_policy<P>>;
    
    inline static task_pool_type& get_task_pool(worker_type& /*wk*/) noexcept;
};

template <typename P>
struct smth_scheduler_policy
    : smth_base_policy<P>
{
    using derived_type = basic_smth_scheduler<smth_scheduler_policy>;
    
    using thread_funcs_type = basic_thread_funcs<smth_thread_policy<P>>;
    using task_pool_type =
        typename P::template task_pool_t<smth_base_policy<P>>;
    
    using tls_key_pool_type = basic_map_tls_key_pool<P>;
};

template <typename P>
using smth_scheduler = basic_smth_scheduler<smth_scheduler_policy<P>>;

template <typename P>
typename smth_thread_policy<P>::task_pool_type&
smth_thread_policy<P>::get_task_pool(
    typename smth_thread_policy<P>::worker_type& /*wk*/
) noexcept {
    return smth_scheduler<P>::get_instance().get_task_pool();
}

template <typename P>
struct smth_uncond_var_policy
    : smth_base_policy<P>
{
    using derived_type = basic_uncond_var<smth_uncond_var_policy>;
};

template <typename P>
struct smth_mcs_mutex_node
{
private:
    using uncond_var_type = basic_uncond_var<smth_uncond_var_policy<P>>;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using atomic_node_ptr_type =
        typename base_ult_itf_type::template atomic<smth_mcs_mutex_node*>;
    
public:
    uncond_var_type         uv;
    atomic_node_ptr_type    next;
};


template <typename P>
struct smth_mutex_policy
    : smth_base_policy<P>
{
private:
    using scheduler_type = smth_scheduler<P>;
    
public:
    using uncond_var_type = basic_uncond_var<smth_uncond_var_policy<P>>;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using atomic_node_ptr_type =
        typename base_ult_itf_type::template atomic<smth_mcs_mutex_node<P>*>;
    
    using mcs_core_type = basic_mcs_core<smth_mutex_policy>;
    using mcs_node_type = smth_mcs_mutex_node<P>;
    
    using mcs_mutex_type = basic_mcs_mutex<smth_mutex_policy>;
    //using mcs_node_pool_type = typename P::mcs_node_pool_type;
    using mcs_node_pool_type =
        typename P::template mcs_node_pool_t<smth_base_policy<P>>;
    
    static mcs_node_pool_type& get_node_pool() noexcept {
        static mcs_node_pool_type pool{
            scheduler_type::get_instance().get_num_workers()
        };
        return pool;
    }
};

template <typename P>
struct smth_cv_policy
    : smth_base_policy<P>
{
    using mcs_mutex_type = basic_pool_mcs_mutex<smth_mutex_policy<P>>;
    using mcs_unique_lock_type = fdn::unique_lock<mcs_mutex_type>;
    
    using uncond_var_type = basic_uncond_var<smth_uncond_var_policy<P>>;
};


template <typename P>
struct smth_barrier_policy {
    using ptrdiff_type = fdn::ptrdiff_t;
    using mutex_type = basic_pool_mcs_mutex<smth_mutex_policy<P>>;
    using cv_type = basic_mcs_cv<smth_cv_policy<P>>;
    using unique_lock_type = fdn::unique_lock<mutex_type>;
};


template <typename P>
struct smth_thread_specific_policy
    : smth_base_policy<P>
{
private:
    using scheduler_type = smth_scheduler<P>;
    using tls_key_pool_type = typename smth_scheduler_policy<P>::tls_key_pool_type;
    
public:
    static tls_key_pool_type& get_tls_key_pool() noexcept {
        return scheduler_type::get_instance().get_tls_key_pool();
    }
};

template <typename P>
struct smth_for_loop_policy
    : smth_base_policy<P>
{
    using thread_type = basic_thread<smth_thread_policy<P>>;
};

template <typename P>
class smth_initializer
{
    using scheduler_type = smth_scheduler<P>;
    using initializer = typename scheduler_type::initializer;
    
public:
    smth_initializer()
        : sched_{}
    {
        sched_.make_workers(2); // FIXME
        
        this->init_ = fdn::make_unique<initializer>(this->sched_);
    }
    
    smth_initializer(const smth_initializer&) = delete;
    smth_initializer& operator = (const smth_initializer&) = delete;
    
private:
    scheduler_type                  sched_;
    fdn::unique_ptr<initializer>    init_;
};


template <typename P>
struct smth_itf
    : basic_for_loop<smth_for_loop_policy<P>>
{
private:
    using base_policy = smth_base_policy<P>;
    using task_desc_type = typename base_policy::task_desc_type;
    using worker_num_type = typename base_policy::worker_num_type;
    using worker_type = typename base_policy::worker_type;
    using scheduler_type = smth_scheduler<P>;
    using base_ult_itf_type = typename P::base_ult_itf_type;
    
public:
    using initializer = smth_initializer<P>;
    
    using thread = basic_thread<smth_thread_policy<P>>;
    
    using uncond_variable = basic_uncond_var<smth_uncond_var_policy<P>>;
    using mutex = basic_pool_mcs_mutex<smth_mutex_policy<P>>;
    using condition_variable = basic_mcs_cv<smth_cv_policy<P>>;
    
    using unique_mutex_lock = fdn::unique_lock<mutex>;
    
    using barrier = basic_cv_barrier<smth_barrier_policy<P>>;
    
    template <typename T>
    using atomic = typename base_ult_itf_type::template atomic<T>;
    
    using spinlock = typename P::spinlock_type;
    
    template <typename VarP>
    using thread_specific =
        basic_map_tls_thread_specific<smth_thread_specific_policy<P>, VarP>;
    
    struct this_thread {
        static void yield() {
            auto& wk = worker_type::get_cur_worker();
            wk.yield();
        }
        
        static task_desc_type* native_handle() noexcept {
            auto& wk = worker_type::get_cur_worker();
            const auto tk = wk.get_cur_task_ref();
            return tk.get_task_desc();
        }
    };
    
    static worker_num_type get_num_workers() noexcept {
        auto& sched = scheduler_type::get_instance();
        return sched.get_num_workers();
    }
    static worker_num_type get_worker_num() noexcept {
        auto& wk = worker_type::get_cur_worker();
        return wk.get_worker_num();
    }
    
    using assert_policy = typename P::assert_policy_type;
    using log_policy = typename P::log_policy_type;
};

} // namespace cmpth
