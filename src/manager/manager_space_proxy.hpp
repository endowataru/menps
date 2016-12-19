
#pragma once

#include "manager_segment_proxy.hpp"

namespace mgdsm {

class manager_space_proxy
{
    typedef segment_id_t                        segment_id_type;
    typedef page_id_t                           page_id_type;
    
public:
    virtual ~manager_space_proxy() /*noexcept*/ = default;
    
    virtual manager_segment_proxy_ptr make_segment_proxy(segment_id_type) = 0;
};

} // namespace mgdsm

