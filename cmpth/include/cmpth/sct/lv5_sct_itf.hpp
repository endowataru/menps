
#pragma once

#include <cmpth/wss/basic_thread_funcs.hpp>
#include <cmpth/wss/basic_thread.hpp>
#include <cmpth/sct/basic_sct_scheduler.hpp>
#include <cmpth/sct/sct_task_pool.hpp>
#include <cmpth/wss/basic_suspended_thread.hpp>

namespace cmpth {

// level 5: thread, scheduler, suspended_thread

template <typename P>
struct lv5_sct_policy
{
    using thread_funcs_type = basic_thread_funcs<lv5_sct_policy>;
    using task_pool_type = sct_task_pool<P>;
    
    using lv4_itf_type = typename P::lv4_itf_type;
    using worker_type = typename lv4_itf_type::worker;
   
    using task_desc_type        = typename lv4_itf_type::task_desc;
    using task_ref_type         = typename lv4_itf_type::task_ref;
    using call_stack_type       = typename lv4_itf_type::call_stack;
    using continuation_type     = typename lv4_itf_type::continuation;
    using unique_task_ptr_type  = typename lv4_itf_type::unique_task_ptr;
    using context_policy_type   = typename lv4_itf_type::context_policy;
    using transfer_type         = typename lv4_itf_type::transfer;
    
    using assert_policy_type    = typename lv4_itf_type::assert_policy;
    using log_policy_type       = typename lv4_itf_type::log_policy;
};

template <typename P>
struct sct_thread_policy
    : lv5_sct_policy<P>
{
private:
    using base = lv5_sct_policy<P>;
    
public:
    using derived_type = basic_thread<sct_thread_policy>;
    using typename base::task_pool_type;
    using typename base::worker_type;
    
    inline static task_pool_type& get_task_pool(worker_type& /*wk*/) noexcept;
};

template <typename P>
struct sct_map_tls_key_pool_policy
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;
    
public:
    using tls_key_type = typename lv4_itf_type::tls_key;
    using spinlock_type = typename lv4_itf_type::spinlock;
};

template <typename P>
struct sct_scheduler_policy
    : lv5_sct_policy<P>
{
private:
    using base = lv5_sct_policy<P>;
    
public:
    using derived_type = basic_sct_scheduler<sct_scheduler_policy>;
    
    using tls_key_pool_type = basic_map_tls_key_pool<sct_map_tls_key_pool_policy<P>>;
    
    using typename base::lv4_itf_type;
    using base_ult_itf_type = typename lv4_itf_type::base_ult_itf;
};

template <typename P>
using sct_scheduler = basic_sct_scheduler<sct_scheduler_policy<P>>;

template <typename P>
typename sct_thread_policy<P>::task_pool_type&
sct_thread_policy<P>::get_task_pool(typename sct_thread_policy<P>::worker_type& /*wk*/) noexcept {
    auto& sched = sct_scheduler<P>::get_instance();
    return sched.get_task_pool();
}

template <typename P>
class sct_initializer
{
    using scheduler_type = basic_sct_scheduler<sct_scheduler_policy<P>>;
    using initializer = typename scheduler_type::initializer;
    
public:
    sct_initializer()
    {
        const char* const num_wks_str = std::getenv("CMPTH_NUM_WORKERS");
        int num_wks = num_wks_str ? std::atoi(num_wks_str) : 1;
        this->sched_.make_workers(num_wks);
        
        this->init_ = fdn::make_unique<initializer>(this->sched_);
    }
    
    sct_initializer(const sct_initializer&) = delete;
    sct_initializer& operator = (const sct_initializer&) = delete;
    
private:
    scheduler_type                  sched_;
    fdn::unique_ptr<initializer>    init_;
};

template <typename P>
struct sct_suspended_thread_policy
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;
    
public:
    using derived_type = basic_suspended_thread<sct_suspended_thread_policy>;
    
    using worker_type       = typename lv4_itf_type::worker;
    using continuation_type = typename lv4_itf_type::continuation;
    using task_desc_type    = typename lv4_itf_type::task_desc;
    
    using assert_policy_type    = typename lv4_itf_type::assert_policy;
    using log_policy_type       = typename lv4_itf_type::log_policy;
};

template <typename P>
struct sct_thread_specific_policy
{
private:
    using scheduler_type = sct_scheduler<P>;
    using tls_key_pool_type = typename sct_scheduler_policy<P>::tls_key_pool_type;
    
    using lv4_itf_type = typename P::lv4_itf_type;
    
public:
    using worker_type = typename lv4_itf_type::worker;
    using tls_map_type = typename lv4_itf_type::tls_map;
    using tls_key_type = typename lv4_itf_type::tls_key;
    
    static tls_key_pool_type& get_tls_key_pool() noexcept {
        return scheduler_type::get_instance().get_tls_key_pool();
    }
};

template <typename P>
struct lv5_sct_itf
    : P::lv4_itf_type
{
    using thread = basic_thread<sct_thread_policy<P>>;
    using scheduler = basic_sct_scheduler<sct_scheduler_policy<P>>;
    using initializer = sct_initializer<P>;
    using suspended_thread = basic_suspended_thread<sct_suspended_thread_policy<P>>;
    
    template <typename VarP>
    using thread_specific =
        basic_map_tls_thread_specific<sct_thread_specific_policy<P>, VarP>;
};

} // namespace cmpth

