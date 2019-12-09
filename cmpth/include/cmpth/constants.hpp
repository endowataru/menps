
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct constants
{
    static constexpr const fdn::size_t default_stack_size = 1 << 16;
    static constexpr const fdn::size_t default_task_pool_threshold = 4;
    static constexpr const fdn::size_t default_mcs_pool_threshold = 64;
    static constexpr const fdn::size_t default_ring_buf_length = 100;
    static constexpr const fdn::size_t default_worker_deque_size = 4096;
    static constexpr bool call_myth_uncond_setup = false;
    static constexpr bool enable_abt_multiple_pools = true;
};

} // namespace cmpth

