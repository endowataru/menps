
#pragma once

#include <mgdsm/segment_ref.hpp>

namespace mgdsm {

class segment_allocator
{
public:
    explicit segment_allocator(sharer_space&);
    
    segment_ref make_segment(
        const mgbase::size_t    size_in_bytes
    ,   const mgbase::size_t    page_size_in_bytes
    );
    
private:
    sharer_space& space_;
};

} // namespace mgdsm

