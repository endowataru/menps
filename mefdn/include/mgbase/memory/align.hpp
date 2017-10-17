
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

inline void* align(
    const mgbase::size_t    alignment
,   const mgbase::size_t    size
,   void*&                  ptr
,   mgbase::size_t&         space
)
{
    typedef mgbase::intptr_t    intptr;
    
    const auto iptr = reinterpret_cast<intptr>(ptr);
    const auto salign = static_cast<intptr>(alignment);
    
    // Calculate the minimum aligned position greater than (ptr).
    const auto aligned = (iptr - 1 + salign) & -salign;
    
    const auto diff =
        static_cast<mgbase::size_t>(
            aligned - iptr
        );
    
    if ((size + diff) > space) {
        return MGBASE_NULLPTR;
    }
    else {
        space -= diff;
        ptr = reinterpret_cast<void*>(aligned);
        return ptr;
    }
}

inline void* align_downward(
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
    
    if (diff > space) {
        return MGBASE_NULLPTR;
    }
    else {
        space -= diff;
        ptr = reinterpret_cast<void*>(aligned);
        return ptr;
    }
}

inline void* align_call_stack(
    const mgbase::size_t    alignment
,   const mgbase::size_t    size
,   void*&                  ptr
,   mgbase::size_t&         space
)
{
    return align_downward(alignment, size, ptr, space);
}

} // namespace mgbase

