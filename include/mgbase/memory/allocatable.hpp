
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

class allocatable
{
public:
    virtual ~allocatable() /*noexcept*/ = default;
    
    virtual void* aligned_alloc(
        mgbase::size_t alignment
    ,   mgbase::size_t size
    )
    = 0;
    
    virtual void free(void* ptr) = 0;
};

} // namespace mgbase

