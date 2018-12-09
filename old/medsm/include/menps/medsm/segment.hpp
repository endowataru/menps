
#pragma once

#include <menps/mefdn/lang.hpp>

namespace menps {
namespace medsm {

class segment
{
public:
    virtual ~segment() /*noexcept*/ = default;
    
    virtual void* get_ptr() const noexcept = 0;
    
    virtual mefdn::size_t get_size_in_bytes() const noexcept = 0;
};

/*
    Notes: allocation is not provided here.
*/

} // namespace medsm
} // namespace menps

