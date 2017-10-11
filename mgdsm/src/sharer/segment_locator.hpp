
#pragma once

#include "mgdsm_common.hpp"

namespace mgdsm {

class segment_locator
{
public:
    virtual ~segment_locator() = default;
    
    virtual void* get_segment_sys_ptr(segment_id_t) MGBASE_NOEXCEPT = 0;
};

} // namespace mgdsm

