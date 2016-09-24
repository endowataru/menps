
#pragma once

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

class my_worker;

struct ult_desc
{
    typedef mgbase::spinlock lock_type;
    mgbase::spinlock lock;
    
    fcontext<my_worker, my_worker> ctx;
    
    ult_state state;
    //bool finished;
    
    bool detached;
    ult_desc* joiner;
    
    void* stack_ptr;
    mgbase::size_t stack_size;
    
    void* result;
};

} // namespace mgult

