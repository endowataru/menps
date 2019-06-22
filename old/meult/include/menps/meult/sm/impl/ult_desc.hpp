
#pragma once

#include "sm_common.hpp"
#include <menps/mefdn/thread/spinlock.hpp>

namespace menps {
namespace meult {
namespace sm {

typedef mefdn::uint64_t    ult_state_underlying_t;

enum class ult_state
    : ult_state_underlying_t
{
    ready
,   blocked
,   finished
};

struct ult_desc
{
    typedef mefdn::spinlock lock_type;
    mefdn::spinlock lock;
    
    ult_state state;
    
    bool detached;
    ult_desc* joiner;
    
    void* stack_ptr;
    mefdn::size_t stack_size;
};

} // namespace sm
} // namespace meult
} // namespace menps

