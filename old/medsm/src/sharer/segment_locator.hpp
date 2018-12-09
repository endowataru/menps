
#pragma once

#include "medsm_common.hpp"

namespace menps {
namespace medsm {

class segment_locator
{
public:
    virtual ~segment_locator() = default;
    
    virtual void* get_segment_sys_ptr(segment_id_t) noexcept = 0;
};

} // namespace medsm
} // namespace menps

