
#pragma once

#include "sm_common.hpp"
#include <mgult/fcontext.hpp>
#include <mgbase/threading/spinlock.hpp>

namespace mgult {

typedef mgbase::uint64_t    ult_state_underlying_t;

enum class ult_state
    : ult_state_underlying_t
{
    ready
,   blocked
,   finished
};

struct ult_desc
{
    typedef mgbase::spinlock lock_type;
    mgbase::spinlock lock;
    
    context_t ctx;
    
    ult_state state;
    
    bool detached;
    ult_desc* joiner;
    
    void* stack_ptr;
    mgbase::size_t stack_size;
};

} // namespace mgult

