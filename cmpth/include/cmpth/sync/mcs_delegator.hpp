
#pragma once

#include <cmpth/sync/basic_sync_delegator.hpp>
#include <cmpth/sync/basic_mcs_queue.hpp>
#include <cmpth/sync/basic_mcs_core.hpp>
#include <cmpth/sync/basic_uncond_thread.hpp>

namespace cmpth {

template <typename P>
class basic_mcs_delegator
    : public basic_sync_delegator<P>
{
    using mcs_node_pool_type = typename P::mcs_node_pool_type;
    
public:
    // for compatibility
    using qdlock_pool_type = mcs_node_pool_type;
    using qdlock_node_type = typename P::sync_node_type;
    
    explicit basic_mcs_delegator(mcs_node_pool_type& pool)
        : pool_(pool)
        // Note: GCC 4.8 cannot use {} for initializing a reference
    { }
    
    mcs_node_pool_type& get_pool() const noexcept {
        return this->pool_;
    }
    
private:
    mcs_node_pool_type& pool_;
};

template <typename UltItf, typename P2>
struct mcs_delegator_node
{
    using uncond_var_type = typename UltItf::uncond_variable;
    using atomic_node_ptr_type =
        typename UltItf::template atomic<mcs_delegator_node*>;
    using delegated_func_type = typename P2::delegated_func_type;
    
    atomic_node_ptr_type    next;
    uncond_var_type*        uv;
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
    using derived_type = basic_mcs_delegator<mcs_delegator_policy>;
    using mcs_core_type = basic_mcs_core<mcs_delegator_policy>;
    using sync_queue_type = basic_mcs_queue<mcs_delegator_policy>;
    
    using sync_node_type = mcs_delegator_node<UltItf, P2>;
    using mcs_node_type = sync_node_type; // TODO: for compatibility
    using atomic_node_ptr_type = typename mcs_node_type::atomic_node_ptr_type;
    using ult_itf_type = UltItf;
    using mcs_node_pool_type =
        basic_ext_return_pool<UltItf, mcs_delegator_pool_policy<UltItf, P2>>;
    
    using qdlock_thread_type = uncond_qdlock_thread<mcs_delegator_policy>;
    
    using assert_policy_type = typename UltItf::assert_policy;
    using log_policy_type = typename UltItf::log_policy;
};

template <typename UltItf, typename P2>
using mcs_delegator =
    basic_mcs_delegator<mcs_delegator_policy<UltItf, P2>>;

} // namespace cmpth

