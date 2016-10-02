
#pragma once

#include "sm_common.hpp"
#include <mgult/fcontext.hpp>
#include <mgbase/threading/spinlock.hpp>

namespace mgult {

enum class ult_state
    : mgbase::uint64_t
{
    ready
,   blocked
,   finished
};

class sm_worker;

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
    
    void* result;
};

} // namespace mgult

