
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace mefdn {

class allocatable
{
public:
    virtual ~allocatable() /*noexcept*/ = default;
    
    virtual void* aligned_alloc(
        mefdn::size_t alignment
    ,   mefdn::size_t size
    )
    = 0;
    
    virtual void free(void* ptr) = 0;
};

} // namespace mefdn
} // namespace menps

