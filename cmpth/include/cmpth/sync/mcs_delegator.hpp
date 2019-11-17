
#pragma once

#include <cmpth/sync/basic_sync_delegator.hpp>
#include <cmpth/sync/basic_mcs_queue.hpp>
#include <cmpth/sync/basic_mcs_core.hpp>

namespace cmpth {

template <typename UltItf, typename P2>
struct mcs_delegator_node
{
    using atomic_node_ptr_type =
        typename UltItf::template atomic<mcs_delegator_node*>;
    using suspended_thread_type = typename UltItf::suspended_thread;
    using delegated_func_type = typename P2::delegated_func_type;
    
    atomic_node_ptr_type    next;
    suspended_thread_type   sth;
    delegated_func_type     func;
};

template <typename UltItf, typename P2>
struct mcs_delegator_pool_policy {
    using element_type = mcs_delegator_node<UltItf, P2>;
    
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& pool) {
        return P2::get_pool_threshold(pool);
    }
    template <typename Node, typename Pool>
    static Node* create(Pool& /*pool*/) {
        return new Node;
    }
    template <typename Node>
    static void destroy(Node* const n) {
        delete n;
    }
};

template <typename UltItf, typename P2>
struct mcs_delegator_policy
{
    using consumer_type = typename P2::consumer_type;
    
    using mcs_core_type = basic_mcs_core<mcs_delegator_policy>;
    using sync_queue_type = basic_mcs_queue<mcs_delegator_policy>;
    
    using sync_node_type = mcs_delegator_node<UltItf, P2>;
    using mcs_node_type = sync_node_type; // TODO: for compatibility
    using atomic_node_ptr_type = typename sync_node_type::atomic_node_ptr_type;
    using base_ult_itf_type = UltItf;
    using mcs_node_pool_type =
        basic_ext_return_pool<UltItf, mcs_delegator_pool_policy<UltItf, P2>>;

    
    using assert_policy_type = typename UltItf::assert_policy;
    using log_aspect_type = typename UltItf::log_aspect;
    
    static const bool prefer_execute_critical = true;
};

template <typename UltItf, typename P2>
using mcs_delegator =
    basic_sync_delegator<mcs_delegator_policy<UltItf, P2>>;

} // namespace cmpth

