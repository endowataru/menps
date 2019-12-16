
#pragma once

#include <cmpth/sync/basic_mcs_core.hpp>
#include <cmpth/sync/basic_mcs_mutex.hpp>

namespace cmpth {

template <typename P>
struct basic_sct_mcs_mutex_node
{
private:
    using base_ult_itf_type = typename P::base_ult_itf_type;
    
public:
    using suspended_thread_type = typename P::suspended_thread_type;
    using atomic_node_ptr_type =
        typename base_ult_itf_type::template atomic<basic_sct_mcs_mutex_node*>;
    
    suspended_thread_type   sth;
    atomic_node_ptr_type    next;
};

template <typename P>
struct basic_sct_mcs_mutex_core_policy {
    using mcs_core_type = basic_mcs_core<basic_sct_mcs_mutex_core_policy>;
    using mcs_node_type = basic_sct_mcs_mutex_node<P>;
    
    using atomic_node_ptr_type = typename mcs_node_type::atomic_node_ptr_type;
    
    using assert_policy_type = typename P::assert_policy_type;
    using log_aspect_type = typename P::log_aspect_type;
};

template <typename P>
struct basic_sct_mcs_mutex_pool_policy {
    using element_type = basic_sct_mcs_mutex_node<P>;
    
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

    using worker_num_type = typename P::worker_num_type;
    static worker_num_type get_num_workers() noexcept {
        return P::get_num_workers();
    }
};

template <typename P>
struct basic_sct_mcs_mutex_policy
    : basic_sct_mcs_mutex_core_policy<P>
{
    using worker_type = typename P::worker_type;
    using suspended_thread_type = typename P::suspended_thread_type;
    
    using mcs_node_pool_type =
        typename P::template memory_pool_t<basic_sct_mcs_mutex_pool_policy<P>>;
    
    static mcs_node_pool_type& get_node_pool() {
        // TODO: move singleton to manager
        static mcs_node_pool_type pool;
        return pool;
    }
};

template <typename P>
using basic_sct_mcs_mutex = basic_mcs_mutex<basic_sct_mcs_mutex_policy<P>>;

} // namespace cmpth

