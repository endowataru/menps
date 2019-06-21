
#pragma once

#include <cmpth/sync/basic_mcs_core.hpp>
#include <cmpth/sync/basic_mcs_mutex.hpp>

namespace cmpth {

template <typename P>
struct sct_mcs_mutex_node
{
private:
    using lv5_itf_type = typename P::lv5_itf_type;
    using base_ult_itf_type = typename lv5_itf_type::base_ult_itf;
    
public:
    using uncond_var_type = typename lv5_itf_type::uncond_variable;
    using atomic_node_ptr_type =
        typename base_ult_itf_type::template atomic<sct_mcs_mutex_node*>;
    
    uncond_var_type         uv;
    atomic_node_ptr_type    next;
};

template <typename P>
struct sct_mcs_mutex_core_policy
{
private:
    using lv5_itf_type = typename P::lv5_itf_type;
    
public:
    using mcs_core_type = basic_mcs_core<sct_mcs_mutex_core_policy>;
    using mcs_node_type = sct_mcs_mutex_node<P>;
    
    using uncond_var_type = typename mcs_node_type::uncond_var_type;
    using atomic_node_ptr_type = typename mcs_node_type::atomic_node_ptr_type;
    
    using assert_policy_type = typename lv5_itf_type::assert_policy;
    using log_policy_type = typename lv5_itf_type::log_policy;
};

template <typename P>
struct sct_mcs_mutex_pool_policy
{
    using element_type = sct_mcs_mutex_node<P>;
    
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& pool) {
        return P::get_mcs_mutex_pool_threshold(pool);
    }
    template <typename Node, typename Pool>
    static Node* create(Pool& /*pool*/) {
        return new Node;
    }
    template <typename Node>
    static void destroy(Node* const n) noexcept {
        delete n;
    }
};

template <typename P>
struct sct_mcs_mutex_policy
    : sct_mcs_mutex_core_policy<P>
{
private:
    using lv5_itf_type = typename P::lv5_itf_type;
    
public:
    using worker_type = typename lv5_itf_type::worker;
    
    using mcs_node_pool_type =
        typename lv5_itf_type::template pool_t<sct_mcs_mutex_pool_policy<P>>;
    
    static mcs_node_pool_type& get_node_pool() {
        static mcs_node_pool_type pool;
        return pool;
    }
};

template <typename P>
using sct_mcs_mutex = basic_mcs_mutex<sct_mcs_mutex_policy<P>>;

} // namespace cmpth

