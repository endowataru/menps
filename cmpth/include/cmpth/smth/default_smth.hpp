
#pragma once

#include <cmpth/smth/smth.hpp>
#include <cmpth/pool/basic_return_pool.hpp>
#include <cmpth/ctx/x86_64_context_policy.hpp>
#include <cmpth/wrap/klt_itf.hpp>
#include <cmpth/smth/smth_task_pool.hpp>
#include <cmpth/smth/smth_mcs_node_pool.hpp>
#include <unordered_map>

namespace cmpth {

struct default_smth_common_policy
{
    static const bool is_debug =
        #ifdef CMPTH_DEBUG
        true;
        #else
        false;
        #endif
    
    using assert_policy_type = assert_policy<is_debug>;
    using log_policy_type = log_policy<is_debug>;
    
    using base_ult_itf_type = klt_itf;
};

struct default_smth_spinlock_policy
    : default_smth_common_policy
{
    using typename default_smth_common_policy::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
};

using default_smth_spinlock = basic_spinlock<default_smth_spinlock_policy>;

struct default_smth_policy;

template <typename P>
struct default_smth_pool_policy
    : default_smth_common_policy
{
    using derived_type = smth_task_pool<default_smth_pool_policy>;
    using pool_base_type = basic_return_pool<default_smth_pool_policy>;
    
    using element_type = smth_task_desc<smth_base_policy<default_smth_policy>>;
    using spinlock_type = default_smth_spinlock;
    
    static fdn::size_t get_pool_threshold(
        const basic_return_pool<default_smth_pool_policy>& /*pool*/
    ) noexcept {
        return 4; // TODO
    }
    
    static fdn::size_t get_default_stack_size() {
        return 16 << 10;
    }
    
    using worker_type = typename P::worker_type;
    using continuation_type = typename P::continuation_type;
    using call_stack_type = typename P::call_stack_type;
    using task_ref_type = typename P::task_ref_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
};

template <typename P>
struct default_smth_mcs_node_pool_policy
    : default_smth_common_policy
{
    using derived_type = smth_mcs_node_pool<default_smth_mcs_node_pool_policy>;
    using pool_base_type = basic_return_pool<default_smth_mcs_node_pool_policy>;
    
    //using element_type = typename smth_mutex_policy<P>::mcs_node_type;
    using element_type = smth_mcs_mutex_node<default_smth_policy>;
    using spinlock_type = default_smth_spinlock;
    
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& /*pool*/) noexcept {
        return 4; // TODO
    }
    
    using worker_type = typename P::worker_type;
};

struct default_smth_wd_policy
    : default_smth_common_policy
{
    using continuation_type = smth_continuation<smth_base_policy<default_smth_policy>>;
    using spinlock_type = default_smth_spinlock;
};


struct default_smth_policy
    : default_smth_common_policy
{
    using worker_deque_type = smth_worker_deque<default_smth_wd_policy>;
    using context_policy_type = x86_64_context_policy<default_smth_wd_policy>;
    using task_mutex_type = default_smth_spinlock;
    
    template <typename P>
    using task_pool_t = smth_task_pool<default_smth_pool_policy<P>>;
    
    /*using mcs_node_pool_type =
        smth_mcs_node_pool<default_smth_mcs_node_pool_policy<P>>;*/
    template <typename P>
    using mcs_node_pool_t = smth_mcs_node_pool<default_smth_mcs_node_pool_policy<P>>;
    
    using spinlock_type = default_smth_spinlock;
    
    using tls_key_type = fdn::size_t;
    using tls_map_impl_type = std::unordered_map<tls_key_type, void*>;
};

using default_smth_itf = smth_itf<default_smth_policy>;

} // namespace cmpth

