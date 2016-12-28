
#pragma once

#include <mgdsm/segment.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class segment_ref
{
public:
    explicit segment_ref(segment* const seg)
        : seg_(seg)
    { }
    
    void* get_ptr() const MGBASE_NOEXCEPT
    {
        return seg_->get_ptr();
    }
    
    mgbase::size_t get_size_in_bytes() const MGBASE_NOEXCEPT
    {
        return seg_->get_size_in_bytes();
    }
    
private:
    mgbase::unique_ptr<segment> seg_;
};

} // namespace mgdsm

