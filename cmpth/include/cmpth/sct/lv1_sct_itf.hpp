
#pragma once

#include <cmpth/wss/basic_unique_task_ptr.hpp>
#include <cmpth/sct/sct_call_stack.hpp>
#include <cmpth/sct/sct_continuation.hpp>
#include <cmpth/sct/sct_task_ref.hpp>
#include <cmpth/tls/basic_map_tls.hpp>
#include <cmpth/wrap/atomic_itf_base.hpp>

namespace cmpth {

// level 1: task_desc

template <typename P>
struct sct_task_desc
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
    bool                is_root;
    continuation_type   joiner;
    void*               stk_top;
    void*               stk_bottom;
    context_type        ctx;
    tls_map_type        tls;
};

template <typename P>
struct lv1_sct_policy
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using context_policy_type = typename P::context_policy_type;
    using task_mutex_type = typename P::task_mutex_type;
    
    using assert_policy_type = typename P::assert_policy_type;
    using log_policy_type = typename P::log_policy_type;
    
    using task_desc_type = sct_task_desc<lv1_sct_policy>;
    using task_ref_type = sct_task_ref<lv1_sct_policy>;
    using call_stack_type = sct_call_stack<lv1_sct_policy>;
    using continuation_type = sct_continuation<lv1_sct_policy>;
    using unique_task_ptr_type = basic_unique_task_ptr<lv1_sct_policy>;
    
    // TODO: There is a circular dependency to an incomplte type here.
    using worker_type = typename P::worker_type;
    
    using context_type =
        typename context_policy_type::template context<worker_type*>;
    using transfer_type =
        typename context_policy_type::template transfer<worker_type*>;
    
    using tls_map_type = basic_map_tls_map<lv1_sct_policy>;
    using tls_key_type = typename P::tls_key_type;
    using tls_map_impl_type = typename P::tls_map_impl_type;
};

template <typename P>
struct lv1_sct_itf
    : atomic_itf_base // TODO
{
private:
    using lv1_policy = lv1_sct_policy<P>;
    
public:
    using spinlock = typename P::spinlock_type;
    
    using task_desc         = typename lv1_policy::task_desc_type;
    using task_ref          = typename lv1_policy::task_ref_type;
    using call_stack        = typename lv1_policy::call_stack_type;
    using continuation      = typename lv1_policy::continuation_type;
    using unique_task_ptr   = typename lv1_policy::unique_task_ptr_type;
    
    using context_policy    = typename lv1_policy::context_policy_type;
    using context           = typename lv1_policy::context_type;
    using transfer          = typename lv1_policy::transfer_type;
    
    using base_ult_itf      = typename lv1_policy::base_ult_itf_type;
    using tls_key           = typename lv1_policy::tls_key_type;
    
    using assert_policy     = typename lv1_policy::assert_policy_type;
    using log_policy        = typename lv1_policy::log_policy_type;
};

} // namespace cmpth

