
#pragma once

#include <mgbase/memory_order.hpp>

namespace mgbase {

MGBASE_ALWAYS_INLINE void atomic_thread_fence(const memory_order /*order*/)
{
    asm volatile("": : :"memory");
    
    __sync_synchronize();
}

} // namespace mgbase

