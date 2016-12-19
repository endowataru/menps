
#pragma once

#include <mgdsm/space.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class space_ref
{
public:
    explicit space_ref(space* const sp)
        : sp_(sp)
    { }
    
    segment_ref make_segment(
        const mgbase::size_t    size_in_bytes
    ,   const mgbase::size_t    page_size_in_bytes
    ) {
        return sp_->make_segment(size_in_bytes, page_size_in_bytes);
    }
    
private:
    mgbase::unique_ptr<space> sp_;
};

} // namespace mgdsm

