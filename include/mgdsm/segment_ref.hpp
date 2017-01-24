
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
    
    segment_ref(const segment_ref&) = delete;
    segment_ref& operator = (const segment_ref&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_1(segment_ref, seg_)
    
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

