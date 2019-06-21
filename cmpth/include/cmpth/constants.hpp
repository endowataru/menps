
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

struct constants
{
    static constexpr const fdn::size_t default_stack_size = 1 << 16;
    static constexpr const fdn::size_t default_task_pool_threshold = 4;
    static constexpr const fdn::size_t default_mcs_pool_threshold = 4;
};

} // namespace cmpth

