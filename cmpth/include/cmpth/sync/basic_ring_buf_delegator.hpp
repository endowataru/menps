
#pragma once

#include <cmpth/sync/basic_sync_delegator.hpp>
#include <cmpth/sync/basic_ring_buf_queue.hpp>
#include <cmpth/sync/basic_ring_buf_core.hpp>

namespace cmpth {

template <typename P>
class basic_ring_buf_delegator
    : public basic_sync_delegator<P>
{
public:
    // for compatibility
    struct qdlock_pool_type { };
    using qdlock_node_type = typename P::sync_node_type;

    explicit basic_ring_buf_delegator(qdlock_pool_type&) { }
};

template <typename UltItf, typename P2>
struct ring_buf_delegator_node
{
    using atomic_bool_type = typename UltItf::template atomic<bool>;
    using uncond_var_type = typename UltItf::uncond_variable;
    using delegated_func_type = typename P2::delegated_func_type;
    
    atomic_bool_type        ready;
    uncond_var_type*        uv;
    delegated_func_type     func;
};

template <typename UltItf, typename P2>
struct ring_buf_delegator_policy
{
    using derived_type = basic_ring_buf_delegator<ring_buf_delegator_policy>;
    using sync_queue_type = basic_ring_buf_queue<ring_buf_delegator_policy>;
    using ring_buf_core_type = basic_ring_buf_core<ring_buf_delegator_policy>;

    using sync_node_type = ring_buf_delegator_node<UltItf, P2>;
    using qdlock_thread_type = uncond_qdlock_thread<ring_buf_delegator_policy>;

    using ult_itf_type = UltItf;
    
    using ring_buf_count_type = fdn::int64_t;
    
    static const ring_buf_count_type ring_buf_length = constants::default_ring_buf_length;
    
    using assert_policy_type = typename UltItf::assert_policy;
    using log_policy_type = typename UltItf::log_policy;
};

template <typename UltItf, typename P2>
using ring_buf_delegator =
    basic_ring_buf_delegator<
        ring_buf_delegator_policy<UltItf, P2>
    >;

} // namespace cmpth

