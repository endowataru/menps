
#pragma once

#include <cmpth/sync/basic_mcs_delegator.hpp>
#include <cmpth/sync/basic_mcs_core.hpp>
#include <cmpth/pool/basic_return_pool.hpp>
#include <cmpth/sync/basic_uncond_thread.hpp>

namespace cmpth {

template <typename P>
class basic_pool_mcs_delegator
    : public basic_mcs_delegator<P>
{
public:
    using mcs_node_pool_type = typename P::mcs_node_pool_type;
    // for compatibility
    using qdlock_pool_type = mcs_node_pool_type;
    using qdlock_node_type = typename P::mcs_node_type;
    
    explicit basic_pool_mcs_delegator(mcs_node_pool_type& pool)
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
struct pool_mcs_delegator_node
{
    using uncond_var_type = typename UltItf::uncond_variable;
    using atomic_node_ptr_type =
        typename UltItf::template atomic<pool_mcs_delegator_node*>;
    using delegated_func_type = typename P2::delegated_func_type;
    
    atomic_node_ptr_type    next;
    uncond_var_type*        uv;
    delegated_func_type     func;
};


template <typename UltItf, typename P2>
class pool_mcs_delegator_pool;

template <typename UltItf, typename P2>
struct pool_mcs_delegator_pool_policy {
    using element_type = pool_mcs_delegator_node<UltItf, P2>;
    
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
struct pool_mcs_delegator_policy
{
    using derived_type = basic_pool_mcs_delegator<pool_mcs_delegator_policy>;
    using mcs_core_type = basic_mcs_core<pool_mcs_delegator_policy>;
    using mcs_node_type = pool_mcs_delegator_node<UltItf, P2>;
    using atomic_node_ptr_type = typename mcs_node_type::atomic_node_ptr_type;
    using ult_itf_type = UltItf;
    using mcs_node_pool_type =
        basic_ext_return_pool<UltItf, pool_mcs_delegator_pool_policy<UltItf, P2>>;
    
    using qdlock_thread_type = uncond_qdlock_thread<pool_mcs_delegator_policy>;
    
    using assert_policy_type = typename UltItf::assert_policy;
    using log_policy_type = typename UltItf::log_policy;
};

template <typename UltItf, typename P2>
using pool_mcs_delegator =
    basic_pool_mcs_delegator<
        pool_mcs_delegator_policy<UltItf, P2>
    >;

} // namespace cmpth

