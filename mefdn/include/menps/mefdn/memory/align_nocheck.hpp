
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

inline void* align_downward_nocheck(
    const mefdn::size_t    alignment
,   const mefdn::size_t    size
,   void*&                  ptr
,   mefdn::size_t&         space
)
{
    typedef mefdn::intptr_t    intptr;
    
    const auto iptr = reinterpret_cast<intptr>(ptr);
    const auto salign = static_cast<intptr>(alignment);
    const auto ssize = static_cast<intptr>(size);
    
    // Calculate the maximum aligned position less than (ptr-size).
    const auto aligned = (iptr - ssize) & -salign;
    
    const auto diff =
        static_cast<mefdn::size_t>(
            iptr - aligned
        );
    
    #ifndef MEFDN_DISABLE_ALIGN_CHECK_SIZE
    if (diff > space) {
        return nullptr;
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
    const mefdn::size_t    alignment
,   const mefdn::size_t    size
,   void*&                  ptr
,   mefdn::size_t&         space
)
{
    return align_downward_nocheck(alignment, size, ptr, space);
}

} // namespace mefdn
} // namespace menps

