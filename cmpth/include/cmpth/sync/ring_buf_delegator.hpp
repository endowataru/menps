
#pragma once

#include <cmpth/sync/basic_sync_delegator.hpp>
#include <cmpth/sync/basic_ring_buf_queue.hpp>
#include <cmpth/sync/basic_ring_buf_core.hpp>

namespace cmpth {

template <typename UltItf, typename P2>
struct ring_buf_delegator_node
{
    using atomic_bool_type = typename UltItf::template atomic<bool>;
    using suspended_thread_type = typename UltItf::suspended_thread;
    using consumer_type = typename P2::consumer_type;
    using delegated_func_type = typename consumer_type::delegated_func_type;
    
    atomic_bool_type        ready;
    suspended_thread_type   sth;
    delegated_func_type     func;
};

template <typename UltItf, typename P2>
struct ring_buf_delegator_policy
{
    using consumer_type = typename P2::consumer_type;

    using sync_queue_type = basic_ring_buf_queue<ring_buf_delegator_policy>;
    using ring_buf_core_type = basic_ring_buf_core<ring_buf_delegator_policy>;

    using sync_node_type = ring_buf_delegator_node<UltItf, P2>;

    using base_ult_itf_type = UltItf;
    
    using ring_buf_count_type = fdn::int64_t;
    
    static const ring_buf_count_type ring_buf_length = constants::default_ring_buf_length;
    
    using assert_aspect_type = typename UltItf::assert_aspect;
    using log_aspect_type = typename UltItf::log_aspect;
    
    static const bool prefer_execute_critical = true;
};

template <typename UltItf, typename P2>
using ring_buf_delegator =
    basic_sync_delegator<ring_buf_delegator_policy<UltItf, P2>>;

} // namespace cmpth

