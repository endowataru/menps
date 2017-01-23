
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
    
    #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
    segment_ref(segment_ref&&) MGBASE_NOEXCEPT_DEFAULT = default;
    #else
    segment_ref(segment_ref&& other) MGBASE_NOEXCEPT
        : seg_(mgbase::move(other.seg_))
    { }
    #endif
    
    #ifdef MGBASE_CXX11_MOVE_ASSIGNMENT_DEFAULT_SUPPORTED
    segment_ref& operator = (segment_ref&&) MGBASE_NOEXCEPT_DEFAULT = default;
    #else
    segment_ref& operator = (segment_ref&& other) MGBASE_NOEXCEPT
    {
        this->seg_ = mgbase::move(other.seg_);
        return *this;
    }
    #endif
    
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

