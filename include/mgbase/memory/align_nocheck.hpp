
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

inline void* align_downward_nocheck(
    const mgbase::size_t    alignment
,   const mgbase::size_t    size
,   void*&                  ptr
,   mgbase::size_t&         space
)
{
    typedef mgbase::intptr_t    intptr;
    
    const auto iptr = reinterpret_cast<intptr>(ptr);
    const auto salign = static_cast<intptr>(alignment);
    const auto ssize = static_cast<intptr>(size);
    
    // Calculate the maximum aligned position less than (ptr-size).
    const auto aligned = (iptr - ssize) & -salign;
    
    const auto diff =
        static_cast<mgbase::size_t>(
            iptr - aligned
        );
    
    #ifndef MGBASE_DISABLE_ALIGN_CHECK_SIZE
    if (diff > space) {
        return MGBASE_NULLPTR;
    }
    else
    #endif
    {
        space -= diff;
        ptr = reinterpret_cast<void*>(aligned);
        return ptr;
    }
}

inline void* align_call_stack_nocheck(
    const mgbase::size_t    alignment
,   const mgbase::size_t    size
,   void*&                  ptr
,   mgbase::size_t&         space
)
{
    return align_downward_nocheck(alignment, size, ptr, space);
}

} // namespace mgbase

