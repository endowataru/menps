
#pragma once

#include "manager_segment_proxy.hpp"

namespace menps {
namespace medsm {

class manager_space
{
    typedef segment_id_t    segment_id_type;
    typedef page_id_t       page_id_type;
    
public:
    virtual ~manager_space() /*noexcept*/ = default;
    
    struct segment_conf {
        mefdn::size_t  num_pages;
        mefdn::size_t  page_size;
        mefdn::size_t  block_size;
    };
    
    virtual void make_segment(segment_id_type, const segment_conf&) = 0;
    
    virtual manager_segment_proxy_ptr make_segment_proxy(segment_id_type) = 0;
};

} // namespace medsm
} // namespace menps

