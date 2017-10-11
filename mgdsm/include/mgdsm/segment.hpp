
#pragma once

#include <mgbase/lang.hpp>

namespace mgdsm {

class segment
{
public:
    virtual ~segment() /*noexcept*/ = default;
    
    virtual void* get_ptr() const MGBASE_NOEXCEPT = 0;
    
    virtual mgbase::size_t get_size_in_bytes() const MGBASE_NOEXCEPT = 0;
};

/*
    Notes: allocation is not provided here.
*/

} // namespace mgdsm

